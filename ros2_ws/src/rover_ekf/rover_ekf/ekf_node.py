#!/usr/bin/env python3
import math
import numpy as np
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu, NavSatFix
from geometry_msgs.msg import Vector3, Twist, TransformStamped
from nav_msgs.msg import Odometry
from std_msgs.msg import Float32
from tf2_ros import TransformBroadcaster


def wrap_angle_rad(angle: float) -> float:
    """Wrap angle in radians to [-pi, pi]."""
    return (angle + math.pi) % (2.0 * math.pi) - math.pi


def euler_rad_to_quaternion(roll: float, pitch: float, yaw: float):
    """Convert euler angles in radians to quaternion (x, y, z, w)."""
    cy = math.cos(yaw * 0.5)
    sy = math.sin(yaw * 0.5)
    cp = math.cos(pitch * 0.5)
    sp = math.sin(pitch * 0.5)
    cr = math.cos(roll * 0.5)
    sr = math.sin(roll * 0.5)

    qw = cr * cp * cy + sr * sp * sy
    qx = sr * cp * cy - cr * sp * sy
    qy = cr * sp * cy + sr * cp * sy
    qz = cr * cp * sy - sr * sp * cy

    return float(qx), float(qy), float(qz), float(qw)


class RoverEKFNode(Node):
    """
    ROS 2 Extended Kalman Filter (EKF) Node fusing:
    1. Wheel Odometry (/cmd_vel or motor topics)
    2. IMU Orientation & Angular Rates (/imu/euler & /imu/data_raw)
    3. GPS Absolute Position (/gps/fix)
    
    5D State Vector x = [X, Y, theta, v, omega]^T
    """

    def __init__(self):
        super().__init__('rover_ekf_node')

        # Parameters
        self.declare_parameter('frequency', 20.0)
        self.declare_parameter('topic_imu_euler', '/imu/euler')
        self.declare_parameter('topic_imu_raw', '/imu/data_raw')
        self.declare_parameter('topic_gps_fix', '/gps/fix')
        self.declare_parameter('topic_cmd_vel', '/cmd_vel')
        self.declare_parameter('topic_ekf_odom', '/ekf/odometry')
        self.declare_parameter('topic_ekf_euler', '/ekf/filtered_euler')
        self.declare_parameter('frame_id', 'odom')
        self.declare_parameter('child_frame_id', 'base_link')

        self.freq = float(self.get_parameter('frequency').value)
        self.dt = 1.0 / max(self.freq, 1.0)
        self.topic_imu_euler = str(self.get_parameter('topic_imu_euler').value)
        self.topic_imu_raw = str(self.get_parameter('topic_imu_raw').value)
        self.topic_gps_fix = str(self.get_parameter('topic_gps_fix').value)
        self.topic_cmd_vel = str(self.get_parameter('topic_cmd_vel').value)
        self.topic_ekf_odom = str(self.get_parameter('topic_ekf_odom').value)
        self.topic_ekf_euler = str(self.get_parameter('topic_ekf_euler').value)
        self.frame_id = str(self.get_parameter('frame_id').value)
        self.child_frame_id = str(self.get_parameter('child_frame_id').value)

        # 5D EKF State: x = [X, Y, theta, v, omega]^T
        self.x = np.zeros(5)
        
        # State Covariance Matrix P (5x5)
        self.P = np.diag([1.0, 1.0, 0.1, 0.5, 0.1])

        # Process Noise Covariance Q (5x5)
        self.Q = np.diag([0.05, 0.05, 0.01, 0.1, 0.05])

        # Measurement Noise Covariances R
        self.R_imu = np.diag([0.01, 0.005])   # [theta_imu, omega_imu]
        self.R_wheel = np.array([[0.08]])      # [v_wheel]
        self.R_gps = np.diag([1.5, 1.5])      # [X_gps, Y_gps] in meters

        # Sensor inputs
        self.gps_origin = None  # (lat0, lon0, alt0)
        self.last_imu_roll = 0.0
        self.last_imu_pitch = 0.0

        # Subscriptions
        self.sub_imu_euler = self.create_subscription(
            Vector3, self.topic_imu_euler, self.imu_euler_callback, 10
        )
        self.sub_imu_raw = self.create_subscription(
            Imu, self.topic_imu_raw, self.imu_raw_callback, 10
        )
        self.sub_gps = self.create_subscription(
            NavSatFix, self.topic_gps_fix, self.gps_fix_callback, 10
        )
        self.sub_cmd_vel = self.create_subscription(
            Twist, self.topic_cmd_vel, self.cmd_vel_callback, 10
        )

        # Motor velocity subscriptions for direct wheel odometry
        self.sub_fl_motor = self.create_subscription(Float32, '/motor/front_left', self.fl_motor_cb, 10)
        self.sub_fr_motor = self.create_subscription(Float32, '/motor/front_right', self.fr_motor_cb, 10)
        self.sub_rl_motor = self.create_subscription(Float32, '/motor/rear_left', self.rl_motor_cb, 10)
        self.sub_rr_motor = self.create_subscription(Float32, '/motor/rear_right', self.rr_motor_cb, 10)

        self.fl_v, self.fr_v, self.rl_v, self.rr_v = 0.0, 0.0, 0.0, 0.0

        # Publishers & TF Broadcaster
        self.pub_odom = self.create_publisher(Odometry, self.topic_ekf_odom, 10)
        self.pub_euler = self.create_publisher(Vector3, self.topic_ekf_euler, 10)
        self.tf_broadcaster = TransformBroadcaster(self)

        # Timer loop for EKF Prediction & Output
        self.timer = self.create_timer(self.dt, self.timer_ekf_loop)

        self.get_logger().info(
            f"Rover EKF Sensor Fusion Node active @ {self.freq} Hz. "
            f"Fusing Wheel Odometry, IMU ({self.topic_imu_euler}), and GPS ({self.topic_gps_fix})."
        )

    def fl_motor_cb(self, msg): self.fl_v = msg.data
    def fr_motor_cb(self, msg): self.fr_v = msg.data
    def rl_motor_cb(self, msg): self.rl_v = msg.data
    def rr_motor_cb(self, msg): self.rr_v = msg.data

    def imu_euler_callback(self, msg: Vector3):
        """Measurement update for IMU Orientation (Yaw, Roll, Pitch)."""
        self.last_imu_roll = math.radians(msg.x)
        self.last_imu_pitch = math.radians(msg.y)
        imu_yaw_rad = math.radians(msg.z)

        # Update theta_yaw state using EKF Innovation
        z = np.array([imu_yaw_rad, self.x[4]])  # [theta, omega]
        H = np.array([
            [0, 0, 1, 0, 0],
            [0, 0, 0, 0, 1]
        ])
        
        y = z - H @ self.x
        y[0] = wrap_angle_rad(y[0])

        S = H @ self.P @ H.T + self.R_imu
        K = self.P @ H.T @ np.linalg.inv(S)
        self.x = self.x + K @ y
        self.x[2] = wrap_angle_rad(self.x[2])
        self.P = (np.eye(5) - K @ H) @ self.P

    def imu_raw_callback(self, msg: Imu):
        """Measurement update for IMU Angular Velocity."""
        omega_z = msg.angular_velocity.z
        z = np.array([self.x[2], omega_z])
        H = np.array([
            [0, 0, 1, 0, 0],
            [0, 0, 0, 0, 1]
        ])
        y = z - H @ self.x
        y[0] = wrap_angle_rad(y[0])
        S = H @ self.P @ H.T + self.R_imu
        K = self.P @ H.T @ np.linalg.inv(S)
        self.x = self.x + K @ y
        self.x[2] = wrap_angle_rad(self.x[2])
        self.P = (np.eye(5) - K @ H) @ self.P

    def cmd_vel_callback(self, msg: Twist):
        """Measurement update from linear velocity command or wheel odometry."""
        v_wheel = msg.linear.x
        self.update_wheel_odometry(v_wheel)

    def update_wheel_odometry(self, v_wheel: float):
        """Apply EKF measurement update for forward speed v_wheel."""
        z = np.array([v_wheel])
        H = np.array([[0, 0, 0, 1, 0]])
        y = z - H @ self.x
        S = H @ self.P @ H.T + self.R_wheel
        K = self.P @ H.T @ np.linalg.inv(S)
        self.x = self.x + K @ y
        self.P = (np.eye(5) - K @ H) @ self.P

    def gps_fix_callback(self, msg: NavSatFix):
        """Measurement update for absolute GPS Position."""
        if msg.status.status < 0 or np.isnan(msg.latitude) or np.isnan(msg.longitude):
            return
        if abs(msg.latitude) < 1e-4 and abs(msg.longitude) < 1e-4:
            return

        # Set reference origin on first valid GPS fix
        if self.gps_origin is None:
            self.gps_origin = (msg.latitude, msg.longitude, msg.altitude)
            self.get_logger().info(
                f"EKF GPS Reference Origin set: Lat={msg.latitude:.6f}, Lon={msg.longitude:.6f}"
            )
            return

        # Convert Lat/Lon to local ENU Cartesian coordinates (x_east, y_north)
        lat0, lon0, _ = self.gps_origin
        r_earth = 6371000.0
        dlat = math.radians(msg.latitude - lat0)
        dlon = math.radians(msg.longitude - lon0)

        x_east = dlon * r_earth * math.cos(math.radians(lat0))
        y_north = dlat * r_earth

        # EKF Measurement Update for GPS position [X, Y]
        z = np.array([x_east, y_north])
        H = np.array([
            [1, 0, 0, 0, 0],
            [0, 1, 0, 0, 0]
        ])
        y = z - H @ self.x
        S = H @ self.P @ H.T + self.R_gps
        K = self.P @ H.T @ np.linalg.inv(S)
        self.x = self.x + K @ y
        self.P = (np.eye(5) - K @ H) @ self.P

    def timer_ekf_loop(self):
        """EKF Prediction Step & State Output Loop."""
        # Calculate wheel odometry forward speed from motor feedback if active
        wheel_speed = ((self.fl_v + self.rl_v) + (self.fr_v + self.rr_v)) * 0.5
        if abs(wheel_speed) > 1e-4:
            self.update_wheel_odometry(wheel_speed)

        # EKF State Prediction: f(x_k, dt)
        X, Y, theta, v, omega = self.x[0], self.x[1], self.x[2], self.x[3], self.x[4]
        
        # Position updated along heading theta
        X_next = X + v * math.cos(theta) * self.dt
        Y_next = Y + v * math.sin(theta) * self.dt
        theta_next = wrap_angle_rad(theta + omega * self.dt)
        v_next = v * 0.98  # slight velocity damping
        omega_next = omega * 0.98

        self.x = np.array([X_next, Y_next, theta_next, v_next, omega_next])

        # State Transition Jacobian Matrix F (5x5)
        F = np.array([
            [1.0, 0.0, -v * math.sin(theta) * self.dt, math.cos(theta) * self.dt, 0.0],
            [0.0, 1.0,  v * math.cos(theta) * self.dt, math.sin(theta) * self.dt, 0.0],
            [0.0, 0.0, 1.0, 0.0, self.dt],
            [0.0, 0.0, 0.0, 0.98, 0.0],
            [0.0, 0.0, 0.0, 0.0, 0.98]
        ])

        # Covariance Prediction: P = F * P * F^T + Q
        self.P = F @ self.P @ F.T + self.Q

        # Publish Fused EKF Outputs
        now = self.get_clock().now().to_msg()
        qx, qy, qz, qw = euler_rad_to_quaternion(self.last_imu_roll, self.last_imu_pitch, self.x[2])

        # 1. Publish Odometry
        odom = Odometry()
        odom.header.stamp = now
        odom.header.frame_id = self.frame_id
        odom.child_frame_id = self.child_frame_id

        odom.pose.pose.position.x = float(self.x[0])
        odom.pose.pose.position.y = float(self.x[1])
        odom.pose.pose.position.z = 0.0

        odom.pose.pose.orientation.x = qx
        odom.pose.pose.orientation.y = qy
        odom.pose.pose.orientation.z = qz
        odom.pose.pose.orientation.w = qw

        odom.twist.twist.linear.x = float(self.x[3])
        odom.twist.twist.angular.z = float(self.x[4])

        pose_cov = np.zeros((6, 6))
        pose_cov[0:2, 0:2] = self.P[0:2, 0:2]
        pose_cov[5, 5] = self.P[2, 2]
        odom.pose.covariance = pose_cov.flatten().tolist()

        self.pub_odom.publish(odom)

        # 2. Publish Filtered Euler Angles (in degrees)
        euler_vec = Vector3()
        euler_vec.x = float(math.degrees(self.last_imu_roll))
        euler_vec.y = float(math.degrees(self.last_imu_pitch))
        euler_vec.z = float(math.degrees(self.x[2]))
        self.pub_euler.publish(euler_vec)

        # 3. Broadcast TF Transform (odom -> base_link)
        tf_stamped = TransformStamped()
        tf_stamped.header.stamp = now
        tf_stamped.header.frame_id = self.frame_id
        tf_stamped.child_frame_id = self.child_frame_id
        tf_stamped.transform.translation.x = float(self.x[0])
        tf_stamped.transform.translation.y = float(self.x[1])
        tf_stamped.transform.translation.z = 0.0
        tf_stamped.transform.rotation.x = qx
        tf_stamped.transform.rotation.y = qy
        tf_stamped.transform.rotation.z = qz
        tf_stamped.transform.rotation.w = qw
        self.tf_broadcaster.sendTransform(tf_stamped)


def main(args=None):
    rclpy.init(args=args)
    node = RoverEKFNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("Rover EKF Node stopped cleanly.")
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
import time
import numpy as np
from pykalman import AdditiveUnscentedKalmanFilter

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Vector3
from std_srvs.srv import Trigger

# adafruit circuitpython i2c / bno085 imports with fallback handling
try:
    from adafruit_extended_bus import ExtendedI2C
    from adafruit_bno08x import (
        BNO_REPORT_ACCELEROMETER,
        BNO_REPORT_GYROSCOPE,
    )
    from adafruit_bno08x.i2c import BNO08X_I2C
    ADAFRUIT_AVAILABLE = True
except ImportError:
    ADAFRUIT_AVAILABLE = False


def euler_deg_to_quaternion(roll_deg: float, pitch_deg: float, yaw_deg: float):
    """convert euler angles in degrees (roll, pitch, yaw) to quaternion (x, y, z, w)."""
    r = np.radians(roll_deg)
    p = np.radians(pitch_deg)
    y = np.radians(yaw_deg)

    cy = np.cos(y * 0.5)
    sy = np.sin(y * 0.5)
    cp = np.cos(p * 0.5)
    sp = np.sin(p * 0.5)
    cr = np.cos(r * 0.5)
    sr = np.sin(r * 0.5)

    qw = cr * cp * cy + sr * sp * sy
    qx = sr * cp * cy - cr * sp * sy
    qy = cr * sp * cy + sr * cp * sy
    qz = cr * cp * sy - sr * sp * cy

    return float(qx), float(qy), float(qz), float(qw)


class KalmanFilterNode(Node):
    """
    ros 2 node implementing an additive unscented kalman filter (aukf) for imu telemetry.
    15-dimensional state: [x, y, z, vx, vy, vz, ax, ay, az, roll, pitch, yaw, gx, gy, gz]
    """

    def __init__(self):
        super().__init__('imu_kalman_filter')

        # declare parameters
        self.declare_parameter('dt', 0.1)
        self.declare_parameter('topic_raw_imu', '/imu/data_raw')
        self.declare_parameter('topic_filtered_imu', '/imu/data_filtered')
        self.declare_parameter('topic_odom', '/imu/odometry')
        self.declare_parameter('topic_filtered_euler', '/imu/filtered_euler')
        self.declare_parameter('frame_id', 'imu_link')
        self.declare_parameter('odom_frame_id', 'odom')
        self.declare_parameter('process_noise', 0.01)
        self.declare_parameter('velocity_damping', 0.95)
        self.declare_parameter('use_direct_sensor', False)
        self.declare_parameter('i2c_bus', 7)
        self.declare_parameter('i2c_address', 0x4A)

        # get parameter values
        self.dt = float(self.get_parameter('dt').value)
        self.topic_raw_imu = str(self.get_parameter('topic_raw_imu').value)
        self.topic_filtered_imu = str(self.get_parameter('topic_filtered_imu').value)
        self.topic_odom = str(self.get_parameter('topic_odom').value)
        self.topic_filtered_euler = str(self.get_parameter('topic_filtered_euler').value)
        self.frame_id = str(self.get_parameter('frame_id').value)
        self.odom_frame_id = str(self.get_parameter('odom_frame_id').value)
        self.process_noise = float(self.get_parameter('process_noise').value)
        self.velocity_damping = float(self.get_parameter('velocity_damping').value)
        self.use_direct_sensor = bool(self.get_parameter('use_direct_sensor').value)
        self.i2c_bus_num = int(self.get_parameter('i2c_bus').value)
        self.i2c_address = int(self.get_parameter('i2c_address').value)

        # build aukf filter parameters matching original specification
        self._init_kalman_filter()

        # publishers
        self.pub_filtered_imu = self.create_publisher(Imu, self.topic_filtered_imu, 10)
        self.pub_odom = self.create_publisher(Odometry, self.topic_odom, 10)
        self.pub_filtered_euler = self.create_publisher(Vector3, self.topic_filtered_euler, 10)

        # subscriber for raw imu data
        self.sub_raw_imu = self.create_subscription(
            Imu,
            self.topic_raw_imu,
            self.raw_imu_callback,
            10
        )

        # services
        self.srv_reset = self.create_service(Trigger, '~/reset_filter', self.handle_reset_filter)
        self.srv_get_state = self.create_service(Trigger, '~/get_state', self.handle_get_state)

        # direct sensor hardware fallback mode if requested
        self.bno = None
        if self.use_direct_sensor:
            self._init_direct_sensor()
            self.timer = self.create_timer(self.dt, self.timer_direct_sensor_callback)

        self.get_logger().info(
            f"Kalman Filter Node initialized. Listening on '{self.topic_raw_imu}', "
            f"publishing filtered outputs to '{self.topic_filtered_imu}' and '{self.topic_odom}'."
        )

    def transition_function(self, state):
        """state transition function (15x15 system model with gravity compensation and velocity damping)."""
        dt = self.dt
        new_state = np.copy(state)

        # gravity compensation on acceleration (subtracting 9.81 m/s^2 along z axis)
        accel_motion = np.copy(state[6:9])
        accel_motion[2] -= 9.81

        # apply velocity damping factor (e.g. 0.95) to prevent unbounded velocity crawl from sensor bias
        velocity_damped = state[3:6] * self.velocity_damping

        # position updated by damped velocity and 0.5 * motion_accel * dt^2
        new_state[0:3] += velocity_damped * dt + 0.5 * accel_motion * (dt ** 2)
        # velocity updated by motion_accel * dt
        new_state[3:6] = velocity_damped + accel_motion * dt
        # orientation (roll, pitch, yaw) updated by gyro * dt
        new_state[9:12] += state[12:15] * dt

        return new_state

    def observation_function(self, state):
        """observation function mapping 15d state to 6d measurement [accel, gyro]."""
        accel_part = state[6:9]
        gyro_part = state[12:15]
        return np.concatenate([accel_part, gyro_part])

    def _init_kalman_filter(self):
        """initialize the additive unscented kalman filter matrices and state."""
        self.transition_covariance = np.eye(15) * self.process_noise
        self.observation_covariance = np.diag([1.0, 1.0, 1.0, 0.04, 0.04, 0.04])
        self.initial_state_mean = np.zeros(15)
        # initialize accel_z to 9.81 so initial state matches rest gravity
        self.initial_state_mean[8] = 9.81
        self.initial_state_covariance = np.eye(15) * 100.0

        self.aukf = AdditiveUnscentedKalmanFilter(
            transition_functions=self.transition_function,
            observation_functions=self.observation_function,
            initial_state_mean=self.initial_state_mean,
            initial_state_covariance=self.initial_state_covariance,
            transition_covariance=self.transition_covariance,
            observation_covariance=self.observation_covariance
        )

        self.current_mean = np.copy(self.initial_state_mean)
        self.current_cov = np.copy(self.initial_state_covariance)

    def _init_direct_sensor(self):
        """optionally connect directly to bno085 if direct sensor mode is enabled."""
        if not ADAFRUIT_AVAILABLE:
            self.get_logger().warn("Direct sensor mode requested but Adafruit libraries not available.")
            return

        try:
            i2c = ExtendedI2C(self.i2c_bus_num)
            self.bno = BNO08X_I2C(i2c, address=self.i2c_address)
            self.bno.enable_feature(BNO_REPORT_ACCELEROMETER)
            self.bno.enable_feature(BNO_REPORT_GYROSCOPE)
            self.get_logger().info(f"Direct sensor mode connected to BNO085 on bus {self.i2c_bus_num}.")
        except Exception as e:
            self.get_logger().error(f"Failed direct sensor connection: {e}")
            self.bno = None

    def process_observation(self, accel: np.ndarray, gyro: np.ndarray, timestamp=None):
        """step the unscented kalman filter with an incoming 6d observation [accel, gyro]."""
        observation = np.concatenate([accel, gyro])
        self.current_mean, self.current_cov = self.aukf.filter_update(
            self.current_mean, self.current_cov, observation
        )
        self.publish_filtered_data(timestamp)

    def raw_imu_callback(self, msg: Imu):
        """subscriber callback receiving raw imu topic messages."""
        accel = np.array([
            msg.linear_acceleration.x,
            msg.linear_acceleration.y,
            msg.linear_acceleration.z
        ])
        gyro = np.array([
            msg.angular_velocity.x,
            msg.angular_velocity.y,
            msg.angular_velocity.z
        ])
        self.process_observation(accel, gyro, msg.header.stamp)

    def timer_direct_sensor_callback(self):
        """timer callback for reading directly from hardware if enabled."""
        if self.bno is not None:
            try:
                accel = np.array(self.bno.acceleration)
                gyro = np.array(self.bno.gyro)
                self.process_observation(accel, gyro, self.get_clock().now().to_msg())
            except Exception as e:
                self.get_logger().error(f"Error reading direct sensor: {e}")

    def publish_filtered_data(self, timestamp=None):
        """publish filtered imu, odometry, and euler angle topics."""
        if timestamp is None:
            stamp = self.get_clock().now().to_msg()
        else:
            stamp = timestamp

        pos = self.current_mean[0:3]
        vel = self.current_mean[3:6]
        accel = self.current_mean[6:9]
        euler_deg = self.current_mean[9:12]
        gyro = self.current_mean[12:15]

        qx, qy, qz, qw = euler_deg_to_quaternion(euler_deg[0], euler_deg[1], euler_deg[2])

        # 1. publish filtered imu
        filtered_imu = Imu()
        filtered_imu.header.stamp = stamp
        filtered_imu.header.frame_id = self.frame_id

        filtered_imu.orientation.x = qx
        filtered_imu.orientation.y = qy
        filtered_imu.orientation.z = qz
        filtered_imu.orientation.w = qw

        filtered_imu.angular_velocity.x = float(gyro[0])
        filtered_imu.angular_velocity.y = float(gyro[1])
        filtered_imu.angular_velocity.z = float(gyro[2])

        filtered_imu.linear_acceleration.x = float(accel[0])
        filtered_imu.linear_acceleration.y = float(accel[1])
        filtered_imu.linear_acceleration.z = float(accel[2])

        # populate 3x3 covariance matrices from 15x15 filter covariance matrix
        filtered_imu.linear_acceleration_covariance = self.current_cov[6:9, 6:9].flatten().tolist()
        filtered_imu.angular_velocity_covariance = self.current_cov[12:15, 12:15].flatten().tolist()
        filtered_imu.orientation_covariance = self.current_cov[9:12, 9:12].flatten().tolist()

        self.pub_filtered_imu.publish(filtered_imu)

        # 2. publish odometry
        odom = Odometry()
        odom.header.stamp = stamp
        odom.header.frame_id = self.odom_frame_id
        odom.child_frame_id = self.frame_id

        odom.pose.pose.position.x = float(pos[0])
        odom.pose.pose.position.y = float(pos[1])
        odom.pose.pose.position.z = float(pos[2])

        odom.pose.pose.orientation.x = qx
        odom.pose.pose.orientation.y = qy
        odom.pose.pose.orientation.z = qz
        odom.pose.pose.orientation.w = qw

        odom.twist.twist.linear.x = float(vel[0])
        odom.twist.twist.linear.y = float(vel[1])
        odom.twist.twist.linear.z = float(vel[2])

        odom.twist.twist.angular.x = float(gyro[0])
        odom.twist.twist.angular.y = float(gyro[1])
        odom.twist.twist.angular.z = float(gyro[2])

        # build 6x6 pose covariance matrix (position 3x3 + orientation 3x3)
        pose_cov = np.zeros((6, 6))
        pose_cov[0:3, 0:3] = self.current_cov[0:3, 0:3]
        pose_cov[3:6, 3:6] = self.current_cov[9:12, 9:12]
        odom.pose.covariance = pose_cov.flatten().tolist()

        # build 6x6 twist covariance matrix (linear vel 3x3 + angular vel 3x3)
        twist_cov = np.zeros((6, 6))
        twist_cov[0:3, 0:3] = self.current_cov[3:6, 3:6]
        twist_cov[3:6, 3:6] = self.current_cov[12:15, 12:15]
        odom.twist.covariance = twist_cov.flatten().tolist()

        self.pub_odom.publish(odom)

        # 3. publish filtered euler angles
        euler_vec = Vector3()
        euler_vec.x = float(euler_deg[0])
        euler_vec.y = float(euler_deg[1])
        euler_vec.z = float(euler_deg[2])
        self.pub_filtered_euler.publish(euler_vec)

    # service handlers
    def handle_reset_filter(self, request, response):
        """service callback to reset filter state mean and covariance."""
        self._init_kalman_filter()
        self.get_logger().info("Kalman Filter state reset to initial zero state.")
        response.success = True
        response.message = "Filter state reset successfully."
        return response

    def handle_get_state(self, request, response):
        """service callback to query current state vector."""
        pos = self.current_mean[0:3]
        vel = self.current_mean[3:6]
        accel = self.current_mean[6:9]
        euler = self.current_mean[9:12]
        gyro = self.current_mean[12:15]

        state_summary = (
            f"Position (x,y,z): [{pos[0]:.3f}, {pos[1]:.3f}, {pos[2]:.3f}]\n"
            f"Velocity (vx,vy,vz): [{vel[0]:.3f}, {vel[1]:.3f}, {vel[2]:.3f}]\n"
            f"Accel (ax,ay,az): [{accel[0]:.3f}, {accel[1]:.3f}, {accel[2]:.3f}]\n"
            f"Euler (deg): [R:{euler[0]:.2f}°, P:{euler[1]:.2f}°, Y:{euler[2]:.2f}°]\n"
            f"Gyro (gx,gy,gz): [{gyro[0]:.3f}, {gyro[1]:.3f}, {gyro[2]:.3f}]"
        )
        response.success = True
        response.message = state_summary
        return response


def main(args=None):
    rclpy.init(args=args)
    node = KalmanFilterNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("Kalman Filter node stopped cleanly.")
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()

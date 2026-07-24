#!/usr/bin/env python3
import time
import sys
import numpy as np

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
from geometry_msgs.msg import Vector3
from std_msgs.msg import String
from std_srvs.srv import Trigger, SetBool

# adafruit circuitpython i2c / bno085 imports with fallback handling
try:
    from adafruit_extended_bus import ExtendedI2C
    from adafruit_bno08x import (
        BNO_REPORT_ACCELEROMETER,
        BNO_REPORT_GYROSCOPE,
        BNO_REPORT_ROTATION_VECTOR,
    )
    from adafruit_bno08x.i2c import BNO08X_I2C
    ADAFRUIT_AVAILABLE = True
except ImportError:
    ADAFRUIT_AVAILABLE = False


def quaternion_to_euler_deg(quat_i: float, quat_j: float, quat_k: float, quat_real: float):
    """convert bno085 rotation vector quaternion to roll, pitch, yaw in degrees."""
    sinr_cosp = 2.0 * (quat_real * quat_i + quat_j * quat_k)
    cosr_cosp = 1.0 - 2.0 * (quat_i * quat_i + quat_j * quat_j)
    roll = np.arctan2(sinr_cosp, cosr_cosp)

    sinp = 2.0 * (quat_real * quat_j - quat_k * quat_i)
    if abs(sinp) >= 1.0:
        pitch = np.sign(sinp) * (np.pi / 2.0)
    else:
        pitch = np.arcsin(sinp)

    siny_cosp = 2.0 * (quat_real * quat_k + quat_i * quat_j)
    cosy_cosp = 1.0 - 2.0 * (quat_j * quat_j + quat_k * quat_k)
    yaw = np.arctan2(siny_cosp, cosy_cosp)

    return float(np.degrees(roll)), float(np.degrees(pitch)), float(np.degrees(yaw))


class TelemetryNode(Node):
    """ros 2 node for reading bno085 imu telemetry and publishing topics/services."""

    def __init__(self):
        super().__init__('imu_telemetry')

        # declare ros 2 parameters
        self.declare_parameter('i2c_bus', 7)
        self.declare_parameter('i2c_address', 0x4A)
        self.declare_parameter('publish_rate', 20.0)
        self.declare_parameter('frame_id', 'imu_link')
        self.declare_parameter('sim_mode', False)
        self.declare_parameter('topic_raw_imu', '/imu/data_raw')
        self.declare_parameter('topic_euler', '/imu/euler')
        self.declare_parameter('topic_telemetry_str', '/imu/telemetry_str')

        # declare noise covariance parameters (diagonal variances for raw imu sensor)
        self.declare_parameter('linear_acceleration_covariance_diagonal', [1.0, 1.0, 1.0])
        self.declare_parameter('angular_velocity_covariance_diagonal', [0.04, 0.04, 0.04])
        self.declare_parameter('orientation_covariance_diagonal', [0.01, 0.01, 0.01])

        # read parameters
        self.i2c_bus_num = self.get_parameter('i2c_bus').value
        self.i2c_address = self.get_parameter('i2c_address').value
        self.publish_rate = self.get_parameter('publish_rate').value
        self.frame_id = self.get_parameter('frame_id').value
        self.sim_mode = self.get_parameter('sim_mode').value
        self.topic_raw_imu = self.get_parameter('topic_raw_imu').value
        self.topic_euler = self.get_parameter('topic_euler').value
        self.topic_telemetry_str = self.get_parameter('topic_telemetry_str').value

        self.publishing_enabled = True
        self.bno = None
        self.i2c = None

        # publishers
        self.pub_imu = self.create_publisher(Imu, self.topic_raw_imu, 10)
        self.pub_euler = self.create_publisher(Vector3, self.topic_euler, 10)
        self.pub_str = self.create_publisher(String, self.topic_telemetry_str, 10)

        # services
        self.srv_calibrate = self.create_service(Trigger, '~/calibrate', self.handle_calibrate)
        self.srv_enable = self.create_service(SetBool, '~/enable_publishing', self.handle_enable_publishing)
        self.srv_status = self.create_service(Trigger, '~/get_status', self.handle_get_status)

        # initialize hardware connection
        self._init_hardware()

        # timer callback
        timer_period = 1.0 / max(self.publish_rate, 1.0)
        self.timer = self.create_timer(timer_period, self.timer_callback)

        self.get_logger().info(
            f"IMU Telemetry Node started. Publishing at {self.publish_rate} Hz on topic '{self.topic_raw_imu}' "
            f"(sim_mode={self.sim_mode})."
        )

    def _init_hardware(self):
        """attempt to initialize the bno085 hardware sensor."""
        if self.sim_mode:
            self.get_logger().info("Simulation mode requested. Running without physical hardware.")
            return

        if not ADAFRUIT_AVAILABLE:
            self.get_logger().warn(
                "Adafruit BNO08x libraries not available. Falling back to simulation mode."
            )
            self.sim_mode = True
            return

        try:
            self.i2c = ExtendedI2C(self.i2c_bus_num)
            self.bno = BNO08X_I2C(self.i2c, address=self.i2c_address)
            self.bno.enable_feature(BNO_REPORT_ACCELEROMETER)
            self.bno.enable_feature(BNO_REPORT_GYROSCOPE)
            self.bno.enable_feature(BNO_REPORT_ROTATION_VECTOR)
            self.sim_mode = False
            self.get_logger().info(
                f"Successfully connected to BNO085 on Bus {self.i2c_bus_num} (Address: {hex(self.i2c_address)})."
            )
        except Exception as e:
            self.get_logger().warn(
                f"Failed to connect to BNO085 on Bus {self.i2c_bus_num}: {e}. "
                "Falling back to simulation mode."
            )
            self.sim_mode = True
            self.bno = None
            self.i2c = None

    def read_sensor_data(self):
        """read sensor data from bno085 or generate simulated data if in sim mode."""
        if not self.sim_mode and self.bno is not None:
            try:
                accel_x, accel_y, accel_z = self.bno.acceleration
                gyro_x, gyro_y, gyro_z = self.bno.gyro
                quat_i, quat_j, quat_k, quat_real = self.bno.quaternion
                return accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, quat_i, quat_j, quat_k, quat_real
            except Exception as e:
                self.get_logger().error(f"Error reading BNO085 sensor: {e}. Switching to sim mode.")
                self.sim_mode = True

        # simulated fallback data (gravity on z, zero gyro, identity quat)
        t = self.get_clock().now().nanoseconds * 1e-9
        accel_x = 0.05 * np.sin(t)
        accel_y = 0.05 * np.cos(t)
        accel_z = 9.81 + 0.02 * np.sin(2 * t)

        gyro_x = 0.01 * np.cos(t)
        gyro_y = 0.01 * np.sin(t)
        gyro_z = 0.0

        quat_i, quat_j, quat_k, quat_real = 0.0, 0.0, 0.0, 1.0
        return accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, quat_i, quat_j, quat_k, quat_real

    def timer_callback(self):
        if not self.publishing_enabled:
            return

        accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, quat_i, quat_j, quat_k, quat_real = self.read_sensor_data()
        roll, pitch, yaw = quaternion_to_euler_deg(quat_i, quat_j, quat_k, quat_real)

        now = self.get_clock().now().to_msg()

        # publish raw imu message
        imu_msg = Imu()
        imu_msg.header.stamp = now
        imu_msg.header.frame_id = self.frame_id

        imu_msg.orientation.x = float(quat_i)
        imu_msg.orientation.y = float(quat_j)
        imu_msg.orientation.z = float(quat_k)
        imu_msg.orientation.w = float(quat_real)

        imu_msg.angular_velocity.x = float(gyro_x)
        imu_msg.angular_velocity.y = float(gyro_y)
        imu_msg.angular_velocity.z = float(gyro_z)

        imu_msg.linear_acceleration.x = float(accel_x)
        imu_msg.linear_acceleration.y = float(accel_y)
        imu_msg.linear_acceleration.z = float(accel_z)

        # populate sensor noise covariance matrices (row-major 3x3)
        accel_var = self.get_parameter('linear_acceleration_covariance_diagonal').value
        gyro_var = self.get_parameter('angular_velocity_covariance_diagonal').value
        orient_var = self.get_parameter('orientation_covariance_diagonal').value

        imu_msg.linear_acceleration_covariance = [
            float(accel_var[0]), 0.0, 0.0,
            0.0, float(accel_var[1]), 0.0,
            0.0, 0.0, float(accel_var[2])
        ]
        imu_msg.angular_velocity_covariance = [
            float(gyro_var[0]), 0.0, 0.0,
            0.0, float(gyro_var[1]), 0.0,
            0.0, 0.0, float(gyro_var[2])
        ]
        imu_msg.orientation_covariance = [
            float(orient_var[0]), 0.0, 0.0,
            0.0, float(orient_var[1]), 0.0,
            0.0, 0.0, float(orient_var[2])
        ]

        self.pub_imu.publish(imu_msg)

        # publish euler angles (vector3: roll, pitch, yaw in degrees)
        euler_msg = Vector3()
        euler_msg.x = float(roll)
        euler_msg.y = float(pitch)
        euler_msg.z = float(yaw)
        self.pub_euler.publish(euler_msg)

        # publish formatted telemetry string
        telemetry_output = (
            f"  Linear Acceleration (m/s²):\n"
            f"    X: {accel_x:8.3f} | Y: {accel_y:8.3f} | Z: {accel_z:8.3f}\n\n"
            f"  Angular Velocity (rad/s):\n"
            f"    X: {gyro_x:8.3f} | Y: {gyro_y:8.3f} | Z: {gyro_z:8.3f}\n\n"
            f"  Orientation (Euler Degrees):\n"
            f"    Roll : {roll:7.2f}°\n"
            f"    Pitch: {pitch:7.2f}°\n"
            f"    Yaw  : {yaw:7.2f}°\n"
        )
        str_msg = String()
        str_msg.data = telemetry_output
        self.pub_str.publish(str_msg)

    # service handlers
    def handle_calibrate(self, request, response):
        """service callback to re-initialize hardware sensor."""
        self.get_logger().info("Calibrate / reset hardware connection requested via service...")
        self.sim_mode = self.get_parameter('sim_mode').value
        self._init_hardware()
        response.success = not self.sim_mode
        response.message = "Hardware re-initialized successfully." if response.success else "Failed to initialize hardware; running in simulation mode."
        return response

    def handle_enable_publishing(self, request, response):
        """service callback to enable/disable telemetry stream."""
        self.publishing_enabled = request.data
        status = "enabled" if self.publishing_enabled else "disabled"
        self.get_logger().info(f"Telemetry publishing set to: {status}")
        response.success = True
        response.message = f"Telemetry publishing {status}."
        return response

    def handle_get_status(self, request, response):
        """service callback to get node status."""
        mode_str = "Simulation" if self.sim_mode else "Hardware (BNO085)"
        pub_str = "Active" if self.publishing_enabled else "Paused"
        response.success = True
        response.message = f"Status: {mode_str} | Publishing: {pub_str} | Bus: {self.i2c_bus_num} | Rate: {self.publish_rate} Hz"
        return response


def main(args=None):
    rclpy.init(args=args)
    node = TelemetryNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("Telemetry node stopped cleanly.")
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()

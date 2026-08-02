#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import NavSatFix, NavSatStatus
from std_msgs.msg import String
import serial

def verify_nmea_checksum(line: str) -> bool:
    """Validate NMEA 0183 checksum (*XX)."""
    if not line.startswith('$') or '*' not in line:
        return False
    try:
        data, checksum_str = line[1:].split('*', 1)
        expected_checksum = int(checksum_str[:2], 16)
        calc_checksum = 0
        for char in data:
            calc_checksum ^= ord(char)
        return calc_checksum == expected_checksum
    except ValueError:
        return False

def nmea_to_decimal(raw_val: str, direction: str) -> float:
    """
    Convert NMEA coordinate format (DDMM.MMMM / DDDMM.MMMM) to Decimal Degrees.
    """
    if not raw_val:
        return 0.0
    val = float(raw_val)
    deg = int(val / 100)
    minutes = val - (deg * 100)
    decimal = deg + (minutes / 60.0)
    if direction in ['S', 'W']:
        decimal = -decimal
    return decimal

class GPSBridgeNode(Node):
    def __init__(self):
        super().__init__('gps_bridge_node')

        # Declare ROS 2 parameters
        self.declare_parameter('port', '/dev/ttyUSB0')
        self.declare_parameter('baud', 38400)
        self.declare_parameter('frame_id', 'gps_link')

        self.port = self.get_parameter('port').get_parameter_value().string_value
        self.baud = self.get_parameter('baud').get_parameter_value().integer_value
        self.frame_id = self.get_parameter('frame_id').get_parameter_value().string_value

        # ROS 2 Publishers
        self.fix_pub = self.create_publisher(NavSatFix, '/gps/fix', 10)
        self.raw_nmea_pub = self.create_publisher(String, '/gps/nmea_raw', 10)

        self.buffer = ""

        self.get_logger().info(f"Opening serial port {self.port} at {self.baud} baud...")

        try:
            # Set timeout=0 for non-blocking reads to avoid blocking the ROS 2 executor thread
            self.ser = serial.Serial(self.port, self.baud, timeout=0)
            self.get_logger().info(f"Successfully connected to GPS device at {self.port}")
        except serial.SerialException as e:
            self.get_logger().error(f"Failed to open serial port {self.port}: {e}")
            self.ser = None

        # Polling timer (20 Hz / 50ms)
        self.timer = self.create_timer(0.05, self.read_serial_data)

    def read_serial_data(self):
        if not rclpy.ok() or self.ser is None or not self.ser.is_open:
            return

        try:
            waiting = self.ser.in_waiting
            if waiting > 0:
                raw_bytes = self.ser.read(waiting)
                text = raw_bytes.decode('ascii', errors='ignore')
                self.buffer += text

                while '\n' in self.buffer:
                    line, self.buffer = self.buffer.split('\n', 1)
                    line = line.strip()

                    if not line or not line.startswith('$'):
                        continue

                    # Publish raw NMEA string
                    nmea_msg = String()
                    nmea_msg.data = line
                    self.raw_nmea_pub.publish(nmea_msg)

                    # Validate NMEA checksum
                    if not verify_nmea_checksum(line):
                        continue

                    # Parse relevant NMEA sentences
                    if line.startswith(('$GNGGA', '$GPGGA')):
                        self.parse_gga(line)
                    elif line.startswith(('$GNRMC', '$GPRMC')):
                        self.parse_rmc(line)

        except Exception as e:
            if rclpy.ok():
                self.get_logger().warn(f"Error reading/parsing serial data: {e}")

    def parse_gga(self, line: str):
        parts = line.split(',')
        if len(parts) < 10:
            return

        fix_quality_str = parts[6]
        if not fix_quality_str or int(fix_quality_str) == 0:
            self.get_logger().info("GPS Status: Searching for satellites / No Fix...", throttle_duration_sec=5.0)
            return

        try:
            lat = nmea_to_decimal(parts[2], parts[3])
            lon = nmea_to_decimal(parts[4], parts[5])
            alt = float(parts[9]) if parts[9] else 0.0

            fix_msg = NavSatFix()
            fix_msg.header.stamp = self.get_clock().now().to_msg()
            fix_msg.header.frame_id = self.frame_id
            fix_msg.status.status = NavSatStatus.STATUS_FIX
            fix_msg.latitude = lat
            fix_msg.longitude = lon
            fix_msg.altitude = alt

            if rclpy.ok():
                self.fix_pub.publish(fix_msg)
                self.get_logger().info(f"Published Fix: Lat={lat:.6f}, Lon={lon:.6f}, Alt={alt:.2f}m", throttle_duration_sec=2.0)

        except (ValueError, IndexError) as e:
            if rclpy.ok():
                self.get_logger().warn(f"Failed to parse GGA line: {line} - Error: {e}")

    def parse_rmc(self, line: str):
        parts = line.split(',')
        if len(parts) < 8:
            return

        status = parts[2]
        if status != 'A':
            return

        try:
            lat = nmea_to_decimal(parts[3], parts[4])
            lon = nmea_to_decimal(parts[5], parts[6])

            fix_msg = NavSatFix()
            fix_msg.header.stamp = self.get_clock().now().to_msg()
            fix_msg.header.frame_id = self.frame_id
            fix_msg.status.status = NavSatStatus.STATUS_FIX
            fix_msg.latitude = lat
            fix_msg.longitude = lon

            if rclpy.ok():
                self.fix_pub.publish(fix_msg)

        except (ValueError, IndexError) as e:
            if rclpy.ok():
                self.get_logger().warn(f"Failed to parse RMC line: {line} - Error: {e}")

    def destroy_node(self):
        if hasattr(self, 'ser') and self.ser and self.ser.is_open:
            self.ser.close()
            self.get_logger().info("Closed serial connection.")
        super().destroy_node()

def main(args=None):
    rclpy.init(args=args)
    node = GPSBridgeNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()

if __name__ == '__main__':
    main()

import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import json
import time
import threading
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), 'pq_bms'))
from battery import BatteryInfo

class BatteryMonitorNode(Node):
    def __init__(self):
        super().__init__('battery_monitor_node')
        self.declare_parameter('mac_address', '')
        self.mac_address = self.get_parameter('mac_address').value
        
        self.pub = self.create_publisher(String, '/battery/status', 10)
        
        if not self.mac_address:
            self.get_logger().error("No MAC address provided! Provide via: --ros-args -p mac_address:='XX:XX:XX:XX:XX:XX'")
            # Wait for mac address to be updated maybe? For now we just return.
            return
            
        self.get_logger().info(f"Starting Battery Monitor for MAC: {self.mac_address}")
        
        self.battery = BatteryInfo(self.mac_address, pair_device=False, timeout=15)
        
        self.thread = threading.Thread(target=self.poll_battery, daemon=True)
        self.thread.start()

    def poll_battery(self):
        while rclpy.ok():
            try:
                self.get_logger().debug("Polling battery BMS...")
                self.battery.read_bms()
                if self.battery.error_code == 0:
                    data = self.battery.get_json()
                    msg = String()
                    msg.data = data
                    self.pub.publish(msg)
                else:
                    self.get_logger().warn(f"Failed to read BMS: {self.battery.error_message}")
            except Exception as e:
                self.get_logger().error(f"Error reading battery: {e}")
            
            time.sleep(5.0)

def main(args=None):
    rclpy.init(args=args)
    node = BatteryMonitorNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.try_shutdown()

if __name__ == '__main__':
    main()

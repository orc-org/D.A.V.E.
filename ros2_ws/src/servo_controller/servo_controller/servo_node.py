import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import Jetson.GPIO as GPIO

class ServoNode(Node):
    def __init__(self):
        super().__init__('servo_node')
        
        # Declare parameters for GPIO pins (Board pin numbers)
        self.declare_parameter('servo1_left_pin', 24)
        self.declare_parameter('servo1_right_pin', 23)
        self.declare_parameter('servo2_left_pin', 21)
        self.declare_parameter('servo2_right_pin', 37)
        
        self.s1_l = self.get_parameter('servo1_left_pin').get_parameter_value().integer_value
        self.s1_r = self.get_parameter('servo1_right_pin').get_parameter_value().integer_value
        self.s2_l = self.get_parameter('servo2_left_pin').get_parameter_value().integer_value
        self.s2_r = self.get_parameter('servo2_right_pin').get_parameter_value().integer_value
        
        # Setup GPIO
        GPIO.setmode(GPIO.BOARD)
        GPIO.setup([self.s1_l, self.s1_r, self.s2_l, self.s2_r], GPIO.OUT, initial=GPIO.LOW)
        
        self.subscription = self.create_subscription(
            String,
            'servo_commands',
            self.listener_callback,
            10)
        self.subscription  # prevent unused variable warning

        self.get_logger().info("Servo Node started. Listening to /servo_commands")

    def listener_callback(self, msg):
        cmd = msg.data.lower().strip()
        self.get_logger().info(f'Received command: {cmd}')
        
        # Parse commands like 'servo1_left', 'servo1_right', 'servo1_stop'
        if cmd == 'servo1_left':
            GPIO.output(self.s1_l, GPIO.HIGH)
            GPIO.output(self.s1_r, GPIO.LOW)
        elif cmd == 'servo1_right':
            GPIO.output(self.s1_l, GPIO.LOW)
            GPIO.output(self.s1_r, GPIO.HIGH)
        elif cmd == 'servo1_stop':
            GPIO.output(self.s1_l, GPIO.LOW)
            GPIO.output(self.s1_r, GPIO.LOW)
        elif cmd == 'servo2_left':
            GPIO.output(self.s2_l, GPIO.HIGH)
            GPIO.output(self.s2_r, GPIO.LOW)
        elif cmd == 'servo2_right':
            GPIO.output(self.s2_l, GPIO.LOW)
            GPIO.output(self.s2_r, GPIO.HIGH)
        elif cmd == 'servo2_stop':
            GPIO.output(self.s2_l, GPIO.LOW)
            GPIO.output(self.s2_r, GPIO.LOW)
        elif cmd == 'stop_all':
            GPIO.output(self.s1_l, GPIO.LOW)
            GPIO.output(self.s1_r, GPIO.LOW)
            GPIO.output(self.s2_l, GPIO.LOW)
            GPIO.output(self.s2_r, GPIO.LOW)
        else:
            self.get_logger().warn(f'Unknown command: {cmd}')

    def __del__(self):
        # Cleanup GPIO on exit
        GPIO.cleanup()

def main(args=None):
    rclpy.init(args=args)
    node = ServoNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()

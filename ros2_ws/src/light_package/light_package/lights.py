#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool

import Jetson.GPIO as GPIO



# BCM pin number (CHANGE THIS)
GPIO_LIGHT_PIN = 40


class LightController(Node):

    def __init__(self):
        super().__init__("light_controller")

        GPIO.setmode(GPIO.BCM)
        GPIO.setwarnings(False)

        GPIO.setup(GPIO_LIGHT_PIN, GPIO.OUT)
        GPIO.output(GPIO_LIGHT_PIN, GPIO.LOW)

        self.subscription = self.create_subscription(
            Bool,
            "/lights",
            self.light_callback,
            10
        )

        self.get_logger().info("Light controller ready.")

    def light_callback(self, msg):

        if msg.data:
            GPIO.output(GPIO_LIGHT_PIN, GPIO.HIGH)
            self.get_logger().info("Lights ON")
        else:
            GPIO.output(GPIO_LIGHT_PIN, GPIO.LOW)
            self.get_logger().info("Lights OFF")

    def destroy_node(self):
        GPIO.output(GPIO_LIGHT_PIN, GPIO.LOW)
        GPIO.cleanup()
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)

    node = LightController()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
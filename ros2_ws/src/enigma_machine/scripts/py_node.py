#!/usr/bin/env python3

import rclpy
from enigma_machine.module_to_import import MyNode

def main(args=None):
    rclpy.init(args=args)
    node = MyNode()
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == "__main__":
    main()
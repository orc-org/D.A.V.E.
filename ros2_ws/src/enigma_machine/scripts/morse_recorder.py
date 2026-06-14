#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from enigma_machine_interfaces.msg import Morse
from cv_bridge import CvBridge
import cv2
import time

class MorseRecorderNode(Node):
    def __init__(self):
        super().__init__('morse_recorder')

        self.bridge = CvBridge()

        # Subscribe to standard camera raw image topic
        self.image_sub = self.create_subscription(
            Image,
            '/image_raw',
            self.image_callback,
            10
        )

        # Publish decoded Morse code string
        self.morse_pub = self.create_publisher(
            Morse,
            '/morse_code',
            10
        )

        # State tracking variables for pulse timing
        self.light_durations = []
        self.start_time = None
        self.last_off_time = 0
        self.is_light_on = False
        self.morse_string = ""

        # Thresholds
        self.declare_parameter('brightness_threshold', 250)
        self.declare_parameter('pixel_count_threshold', 9500)
        
        self.brightness_thresh = self.get_parameter('brightness_threshold').value
        self.pixel_thresh = self.get_parameter('pixel_count_threshold').value

        self.get_logger().info('Morse Recorder ROS 2 camera node running.')

    def image_callback(self, msg):
        try:
            # Convert ROS Image to OpenCV BGR frame
            frame = self.bridge.imgmsg_to_cv2(msg, 'bgr8')
        except Exception as e:
            self.get_logger().error(f'Failed to convert image: {str(e)}')
            return

        # Apply grayscale and thresholding to isolate bright flashing light
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        _, thresh = cv2.threshold(gray, self.brightness_thresh, 255, cv2.THRESH_BINARY)
        pixel_count = cv2.countNonZero(thresh)

        now = time.time()

        if pixel_count > self.pixel_thresh:
            if not self.is_light_on:
                self.start_time = now
                self.is_light_on = True
                self.get_logger().info("Light Detected: ON")
                
                # Check for space duration between letters/words
                off_duration = now - self.last_off_time
                if self.light_durations:
                    max_dur = max(self.light_durations)
                    if off_duration > (max_dur * 0.6):
                        self.morse_string += " "  # Space between letters
                        self.publish_current_morse()
        else:
            if self.is_light_on:
                duration = now - self.start_time
                self.light_durations.append(duration)
                self.is_light_on = False
                self.get_logger().info(f"Light Detected: OFF | Duration: {duration:.2f}s")

                # Reconstruct dit (*) or daw (-) based on duration
                if len(self.light_durations) == 1:
                    # Assume first signal is a dot (dit) to initialize baseline
                    self.morse_string += "*"
                else:
                    # Compare to the minimum seen pulse duration
                    min_dur = min(self.light_durations)
                    if duration > (min_dur * 2):
                        self.morse_string += "-"  # Dash (daw)
                    else:
                        self.morse_string += "*"  # Dot (dit)
                
                self.get_logger().info(f"Current Morse String: {self.morse_string}")
                self.publish_current_morse()
                self.last_off_time = now

        # Optionally show image in debug window if running in desktop environment
        # (wrapped in try-catch to not crash if running headless)
        try:
            cv2.imshow('Threshold Feed (Debug)', thresh)
            cv2.waitKey(1)
        except Exception:
            pass

    def publish_current_morse(self):
        msg = Morse()
        msg.message = self.morse_string
        self.morse_pub.publish(msg)


def main(args=None):
    rclpy.init(args=args)
    node = MorseRecorderNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        cv2.destroyAllWindows()
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
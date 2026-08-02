#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from std_msgs.msg import String, Int32
from std_srvs.srv import Trigger
from rcl_interfaces.msg import SetParametersResult
from enigma_machine_interfaces.msg import Morse
from cv_bridge import CvBridge
import cv2
import time
import threading

class MorseRecorderNode(Node):
    def __init__(self):
        super().__init__('morse_recorder')

        self.bridge = CvBridge()

        # Declare ROS 2 Parameters
        self.declare_parameter('source', 'udp')  # Options: 'udp', 'topic', 'camera'
        self.declare_parameter('udp_port', 5001)
        self.declare_parameter('topic', '/image_raw')
        self.declare_parameter('camera_device', '0')
        self.declare_parameter('brightness_threshold', 240)
        self.declare_parameter('pixel_count_threshold', 150)  # Calibrated for 320x180 downscaled resolution

        self.source = self.get_parameter('source').value
        self.udp_port = self.get_parameter('udp_port').value
        self.topic = self.get_parameter('topic').value
        self.camera_device = self.get_parameter('camera_device').value
        self.brightness_thresh = self.get_parameter('brightness_threshold').value
        self.pixel_thresh = self.get_parameter('pixel_count_threshold').value

        # Register Dynamic Parameter Change Callback
        self.add_on_set_parameters_callback(self.on_param_change)

        # Publish decoded Morse code string & live pixel telemetry
        self.morse_pub = self.create_publisher(
            Morse,
            '/morse_code',
            10
        )
        self.str_pub = self.create_publisher(
            String,
            '/morse_code_str',
            10
        )
        self.pixel_pub = self.create_publisher(
            Int32,
            '/morse_recorder/pixel_count',
            10
        )

        # Service to reset string buffer
        self.reset_srv = self.create_service(
            Trigger,
            '/morse_recorder/reset',
            self.handle_reset
        )

        # State tracking variables for pulse timing
        self.light_durations = []
        self.start_time = None
        self.last_off_time = 0
        self.is_light_on = False
        self.morse_string = ""
        self.running = True

        self.get_logger().info(f'Morse Recorder Node initialized (Source: {self.source}).')

        # Select input capture mode
        if self.source == 'topic':
            self.get_logger().info(f'Subscribing to ROS 2 image topic: {self.topic}')
            self.image_sub = self.create_subscription(
                Image,
                self.topic,
                self.image_callback,
                10
            )
        else:
            # Start background thread for direct UDP stream or camera capture
            self.capture_thread = threading.Thread(target=self.run_capture_loop, daemon=True)
            self.capture_thread.start()

    def run_capture_loop(self):
        if self.source == 'udp':
            gst_pipeline = (
                f"udpsrc port={self.udp_port} ! "
                f"application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96 ! "
                f"rtpjitterbuffer latency=0 drop-on-latency=true ! "
                f"rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! "
                f"video/x-raw, format=BGR ! appsink drop=true"
            )
            self.get_logger().info(f'Opening GStreamer UDP receiver pipeline on port {self.udp_port}...')
            cap = cv2.VideoCapture(gst_pipeline, cv2.CAP_GSTREAMER)
        elif self.source == 'camera':
            dev = int(self.camera_device) if self.camera_device.isdigit() else self.camera_device
            self.get_logger().info(f'Opening camera device directly: {dev}')
            cap = cv2.VideoCapture(dev, cv2.CAP_V4L2)
        else:
            self.get_logger().error(f'Unknown source mode: {self.source}')
            return

        if not cap.isOpened():
            self.get_logger().error(f'Failed to open capture stream for source "{self.source}".')
            return

        self.get_logger().info(f'Capture stream opened successfully for source "{self.source}".')

        while rclpy.ok() and self.running:
            ret, frame = cap.read()
            if not ret:
                time.sleep(0.01)
                continue
            self.process_frame(frame)

        cap.release()

    def image_callback(self, msg):
        try:
            # Convert ROS Image to OpenCV BGR frame
            frame = self.bridge.imgmsg_to_cv2(msg, 'bgr8')
            self.process_frame(frame)
        except Exception as e:
            self.get_logger().error(f'Failed to convert image: {str(e)}')

    def on_param_change(self, params):
        for param in params:
            if param.name == 'brightness_threshold':
                self.brightness_thresh = int(param.value)
                self.get_logger().info(f"Dynamic parameter update: brightness_threshold = {self.brightness_thresh}")
            elif param.name == 'pixel_count_threshold':
                self.pixel_thresh = int(param.value)
                self.get_logger().info(f"Dynamic parameter update: pixel_count_threshold = {self.pixel_thresh}")
        return SetParametersResult(successful=True)

    def handle_reset(self, request, response):
        self.reset_buffer()
        response.success = True
        response.message = "Morse buffer reset successfully."
        return response

    def reset_buffer(self):
        self.morse_string = ""
        self.light_durations = []
        self.start_time = None
        self.last_off_time = 0
        self.is_light_on = False
        self.publish_current_morse()
        self.get_logger().info("Morse code buffer reset.")

    def process_frame(self, frame):
        # Downscale frame by ~94% (320x180) for ultra-fast CPU optical thresholding
        small_frame = cv2.resize(frame, (320, 180), interpolation=cv2.INTER_NEAREST)

        # Apply grayscale and thresholding to isolate bright flashing light
        gray = cv2.cvtColor(small_frame, cv2.COLOR_BGR2GRAY)
        _, thresh = cv2.threshold(gray, self.brightness_thresh, 255, cv2.THRESH_BINARY)
        pixel_count = cv2.countNonZero(thresh)

        # Publish live pixel count telemetry for real-time dashboard calibration
        pix_msg = Int32()
        pix_msg.data = int(pixel_count)
        self.pixel_pub.publish(pix_msg)

        now = time.time()

        if pixel_count > self.pixel_thresh:
            if not self.is_light_on:
                # Auto-reset buffer if inactive for > 10 seconds before a new message starts
                if self.last_off_time > 0 and (now - self.last_off_time > 10.0) and self.morse_string:
                    self.get_logger().info("Inactivity timeout (>10s): Auto-clearing old Morse buffer for new message session.")
                    self.reset_buffer()

                self.start_time = now
                self.is_light_on = True
                self.get_logger().info(f"Light Detected: ON | Pixel Count: {pixel_count} (Req: >{self.pixel_thresh})")
                
                # Check for space duration between letters/words
                off_duration = now - self.last_off_time
                if self.light_durations:
                    min_dur = min(self.light_durations)
                    if off_duration > (min_dur * 5.5):
                        if not self.morse_string.endswith("  "):
                            self.morse_string += "  "  # Double space between words
                            self.publish_current_morse()
                    elif off_duration > (min_dur * 1.8):
                        if not self.morse_string.endswith(" "):
                            self.morse_string += " "   # Single space between letters
                            self.publish_current_morse()
        else:
            if self.is_light_on:
                duration = now - self.start_time
                self.light_durations.append(duration)
                if len(self.light_durations) > 20:
                    self.light_durations = self.light_durations[-20:]  # Sliding window of last 20 pulses

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
        try:
            cv2.imshow('Threshold Feed (Debug)', thresh)
            cv2.waitKey(1)
        except Exception:
            pass

    def publish_current_morse(self):
        msg = Morse()
        msg.message = self.morse_string
        self.morse_pub.publish(msg)

        str_msg = String()
        str_msg.data = self.morse_string
        self.str_pub.publish(str_msg)

    def destroy_node(self):
        self.running = False
        super().destroy_node()


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
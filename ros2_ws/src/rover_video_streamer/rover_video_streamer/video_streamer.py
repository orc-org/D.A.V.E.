#!/usr/bin/env python3

import sys
import argparse
import time
import cv2

# try importing ROS 2 dependencies
try:
    import rclpy
    from rclpy.node import Node
    from sensor_msgs.msg import Image
    from cv_bridge import CvBridge, CvBridgeError
    ROS2_AVAILABLE = True
except ImportError:
    ROS2_AVAILABLE = False


class VideoStreamer:
    """Manages the GStreamer pipeline and pushes OpenCV frames to it."""
    
    def __init__(self, host, port, fps, width, height, bitrate, use_mjpeg):
        self.host = host
        self.port = port
        self.fps = fps
        self.width = width
        self.height = height
        self.bitrate = bitrate
        self.use_mjpeg = use_mjpeg
        self.writer = None
        self.init_gst_writer()

    def init_gst_writer(self):
        # start our pipeline with appsrc, specifying that the input is raw BGR frames.
        # then, we use videoconvert to convert to the layout expected by the encoders.
        if self.use_mjpeg:
            # MJPEG pipeline: uses low CPU overhead since it only compresses frame by frame.
            # Good if CPU is highly constrained, but consumes significantly more network bandwidth.
            gst_pipeline = (
                f"appsrc ! video/x-raw, format=BGR ! queue ! videoconvert ! "
                f"jpegenc quality=80 ! rtpjpegpay ! "
                f"udpsink host={self.host} port={self.port} sync=false async=false buffer-size=2097152"
            )
        else:
            # H.264 software encoding (x264enc) with low latency tuning:
            # - tune=zerolatency: disables B-frames and multi-frame lookahead buffering.
            # - speed-preset=ultrafast: minimizes CPU encoding time.
            # - key-int-max=30: sends an I-frame at least every 30 frames (1 second at 30fps) for quick recovery on lossy links.
            # - threads=4: allocates threads to process the encoding rapidly.
            # - config-interval=1: inserts SPS/PPS headers in-band periodically so receivers can decode immediately.
            # - aggregate-mode=zero-latency: groups NAL units into single packets to minimize network overhead and latency.
            # - udpsink sync=false async=false: pushes frames onto the network without waiting for pipeline clock sync.
            # - buffer-size=2097152: 2MB UDP socket send buffer size to prevent kernel-level packet drops.
            gst_pipeline = (
                f"appsrc ! video/x-raw, format=BGR ! queue ! videoconvert ! video/x-raw, format=I420 ! "
                f"x264enc tune=zerolatency bitrate={self.bitrate} speed-preset=ultrafast key-int-max={int(self.fps)} threads=4 ! "
                f"rtph264pay config-interval=1 aggregate-mode=zero-latency ! "
                f"udpsink host={self.host} port={self.port} sync=false async=false buffer-size=2097152"
            )
        
        print(f"Initializing GStreamer VideoWriter pipeline:\n{gst_pipeline}")
        self.writer = cv2.VideoWriter(
            gst_pipeline,
            cv2.CAP_GSTREAMER,
            0,
            float(self.fps),
            (self.width, self.height),
            True
        )
        
        if not self.writer.isOpened():
            print("ERROR: Failed to open GStreamer VideoWriter. Verify that gstreamer plugins are installed.")
            sys.exit(1)
        print("GStreamer VideoWriter initialized successfully.")

    def send_frame(self, frame):
        if frame is None:
            return
        
        # ensure the frame matches the target dimensions
        h, w = frame.shape[:2]
        if w != self.width or h != self.height:
            frame = cv2.resize(frame, (self.width, self.height))
            
        self.writer.write(frame)

    def release(self):
        if self.writer:
            self.writer.release()
            print("GStreamer VideoWriter released.")


if ROS2_AVAILABLE:
    class VideoStreamerROS2Node(Node):
        """ROS 2 wrapper node subscribing to image topics and sending frames to the GStreamer pipeline."""
        
        def __init__(self, args, streamer):
            super().__init__('rover_video_streamer')
            self.args = args
            self.streamer = streamer
            self.bridge = CvBridge()
            
            # subscribe to standard camera raw image topic
            self.subscription = self.create_subscription(
                Image,
                args.topic,
                self.image_callback,
                10  # queue size (best effort or short queue size is better for live video)
            )
            self.get_logger().info(f"Subscribed to ROS 2 topic: {args.topic}")
            self.get_logger().info(f"Streaming to {args.host}:{args.port} at {args.width}x{args.height} @ {args.fps} FPS")

        def image_callback(self, msg):
            try:
                # convert ROS Image message to OpenCV BGR frame
                frame = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
                self.streamer.send_frame(frame)
            except CvBridgeError as e:
                self.get_logger().error(f"CvBridge conversion error: {e}")
            except Exception as e:
                self.get_logger().error(f"Error processing frame: {e}")

        def destroy_node(self):
            self.streamer.release()
            super().destroy_node()


def run_standalone_camera(streamer, device_id):
    """Fallback runner that captures directly from a hardware camera index or device path."""
    print(f"Starting standalone camera stream from device {device_id}...")
    
    try:
        device = int(device_id)
    except ValueError:
        device = device_id
        
    cap = cv2.VideoCapture(device)
    if not cap.isOpened():
        print(f"ERROR: Could not open camera device {device_id}.")
        return

    # request the resolution and fps from v4l2 device
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, streamer.width)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, streamer.height)
    cap.set(cv2.CAP_PROP_FPS, streamer.fps)

    delay = 1.0 / streamer.fps

    try:
        while True:
            t0 = time.time()
            ret, frame = cap.read()
            if not ret:
                print("Capture failed or end of stream. Retrying...")
                time.sleep(0.1)
                continue

            streamer.send_frame(frame)

            # control frame rate
            elapsed = time.time() - t0
            sleep_time = delay - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)
    except KeyboardInterrupt:
        print("\nStopping camera capture stream...")
    finally:
        cap.release()
        streamer.release()


def main(args=None):
    parser = argparse.ArgumentParser(description="Low-Latency Video Streamer for Jetson Orin Nano")
    parser.add_argument('--host', type=str, default='192.168.1.10', help='Base station IP address')
    parser.add_argument('--port', type=int, default=5000, help='UDP port to send stream')
    parser.add_argument('--fps', type=int, default=30, help='Target framerate')
    parser.add_argument('--width', type=int, default=1280, help='Target frame width')
    parser.add_argument('--height', type=int, default=720, help='Target frame height')
    parser.add_argument('--bitrate', type=int, default=2000, help='H.264 bitrate in kbps (x264enc)')
    parser.add_argument('--source', type=str, default='camera', choices=['camera', 'topic'], 
                        help='Input source: "camera" (V4L2) or "topic" (ROS 2 subscriber)')
    parser.add_argument('--device', type=str, default='0', help='Camera device index or V4L2 path (e.g. 0 or /dev/video0)')
    parser.add_argument('--topic', type=str, default='/image_raw', help='ROS 2 image topic to subscribe to')
    parser.add_argument('--mjpeg', action='store_true', help='Use MJPEG compression instead of H.264')
    parser.add_argument('--standalone', action='store_true', help='Force standalone camera mode even if ROS 2 is sourced')

    # resolve arguments
    parsed_args = parser.parse_args(args=args if args is not None else sys.argv[1:])

    # instantiate the core GStreamer video writer
    streamer = VideoStreamer(
        host=parsed_args.host,
        port=parsed_args.port,
        fps=parsed_args.fps,
        width=parsed_args.width,
        height=parsed_args.height,
        bitrate=parsed_args.bitrate,
        use_mjpeg=parsed_args.mjpeg
    )

    if parsed_args.source == 'topic' and not parsed_args.standalone:
        if not ROS2_AVAILABLE:
            print("ERROR: ROS 2 imports failed. Cannot run in topic mode. Use --standalone / --source camera.")
            sys.exit(1)
        
        # initialize ROS 2 and run the subscriber node
        rclpy.init()
        node = VideoStreamerROS2Node(parsed_args, streamer)
        try:
            rclpy.spin(node)
        except KeyboardInterrupt:
            print("\nShutting down ROS 2 node...")
        finally:
            node.destroy_node()
            rclpy.try_shutdown()
    else:
        # run standalone camera grabber
        run_standalone_camera(streamer, parsed_args.device)


if __name__ == '__main__':
    main()

#!/usr/bin/env python3

import sys
import argparse
import time
import cv2
import threading

# try importing ROS 2 dependencies
try:
    import rclpy
    from rclpy.node import Node
    from sensor_msgs.msg import Image
    from cv_bridge import CvBridge, CvBridgeError
    from std_srvs.srv import Trigger
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
        
        # autofocus state variables
        self.focus_subdev = self.find_focus_subdev()
        self.autofocus_in_progress = False
        self.last_frame_sharpness = 0.0
        
        self.init_gst_writer()

    def find_focus_subdev(self):
        """Scans V4L2 subdevices to find the focus controller (VCM)."""
        import glob
        import subprocess
        for dev in glob.glob("/dev/v4l-subdev*"):
            try:
                res = subprocess.run(["v4l2-ctl", "-d", dev, "--list-ctrls"], capture_output=True, text=True)
                if "focus_absolute" in res.stdout:
                    return dev
            except Exception:
                continue
        return None

    def set_focus(self, value):
        """Writes the focus target down to the V4L2 subdevice lens motor."""
        if not self.focus_subdev:
            return
        import subprocess
        try:
            subprocess.run(["v4l2-ctl", "-d", self.focus_subdev, "--set-ctrl", f"focus_absolute={int(value)}"], capture_output=True)
        except Exception as e:
            print(f"[Autofocus] Error setting focus to {value}: {e}")

    def calculate_sharpness(self, frame):
        """Calculates image sharpness using the variance of the Laplacian."""
        try:
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            return cv2.Laplacian(gray, cv2.CV_64F).var()
        except Exception:
            return 0.0

    def trigger_sweep(self):
        """Launches the autofocus sweep thread if not already running."""
        if not self.focus_subdev:
            print("[Autofocus] WARNING: No focus motor subdevice found. Focus sweep aborted.")
            return False
        if self.autofocus_in_progress:
            print("[Autofocus] Focus sweep already in progress.")
            return False
        
        threading.Thread(target=self._autofocus_sweep_thread).start()
        return True

    def _autofocus_sweep_thread(self):
        """Sweeps focus values and sets focus to the sharpest point without stopping the feed."""
        self.autofocus_in_progress = True
        print("[Autofocus] Starting on-the-fly focus sweep...")
        
        focus_min = 100
        focus_max = 900
        coarse_step = 60
        
        best_focus = focus_min
        max_sharpness = 0.0
        
        # coarse sweep
        for val in range(focus_min, focus_max + 1, coarse_step):
            self.set_focus(val)
            # sleep slightly longer than frametime to guarantee a fresh frame is processed by opencv
            time.sleep(0.2)
            
            sharpness = self.last_frame_sharpness
            print(f"[Autofocus] Coarse Focus: {val:3d} | Sharpness: {sharpness:.2f}")
            if sharpness > max_sharpness:
                max_sharpness = sharpness
                best_focus = val
                
        # fine sweep around the coarse peak
        fine_min = max(focus_min, best_focus - 50)
        fine_max = min(focus_max, best_focus + 50)
        fine_step = 15
        
        for val in range(fine_min, fine_max + 1, fine_step):
            self.set_focus(val)
            time.sleep(0.18)
            
            sharpness = self.last_frame_sharpness
            print(f"[Autofocus] Fine Focus: {val:3d} | Sharpness: {sharpness:.2f}")
            if sharpness > max_sharpness:
                max_sharpness = sharpness
                best_focus = val
                
        print(f"[Autofocus] Optimal focus found! Locking focus position at: {best_focus}")
        self.set_focus(best_focus)
        self.autofocus_in_progress = False

    def init_gst_writer(self):
        if self.use_mjpeg:
            # MJPEG pipeline
            gst_pipeline = (
                f"appsrc ! video/x-raw, format=BGR ! queue ! videoconvert ! "
                f"jpegenc quality=80 ! rtpjpegpay ! "
                f"udpsink host={self.host} port={self.port} sync=false async=false buffer-size=2097152"
            )
        else:
            # H.264 software encoding with low latency tuning
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
        
        # compute image sharpness in real-time only during active autofocus sweeps
        if self.autofocus_in_progress:
            self.last_frame_sharpness = self.calculate_sharpness(frame)
            
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
        """ROS 2 node subscribing to image topics and sending frames to the GStreamer pipeline."""
        
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
                10  # queue size (best effort or short queue size is best for live video)
            )
            self.get_logger().info(f"Subscribed to ROS 2 topic: {args.topic}")
            
            # register ROS 2 Service to trigger autofocus on the go
            self.srv = self.create_service(
                Trigger,
                '~/trigger_autofocus',
                self.trigger_autofocus_callback
            )
            self.get_logger().info("Autofocus service registered at '~/trigger_autofocus'")
            self.get_logger().info(f"Streaming to {args.host}:{args.port} at {args.width}x{args.height} @ {args.fps} FPS")

            # optionally autofocus on startup
            if args.autofocus_on_start:
                self.get_logger().info("Queueing autofocus on startup...")
                self.streamer.trigger_sweep()

        def trigger_autofocus_callback(self, request, response):
            success = self.streamer.trigger_sweep()
            if success:
                response.success = True
                response.message = "Autofocus sweep started in background thread."
            else:
                response.success = False
                response.message = "Autofocus failed to start (motor missing or sweep already in progress)."
            return response

        def image_callback(self, msg):
            # convert ros image message to cv2 BGR frame
            try:
                frame = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
                self.streamer.send_frame(frame)
            except CvBridgeError as e:
                self.get_logger().error(f"CvBridge conversion error: {e}")
            except Exception as e:
                self.get_logger().error(f"Error processing frame: {e}")

        def destroy_node(self):
            self.streamer.release()
            super().destroy_node()


def start_udp_trigger_listener(streamer, port=5005):
    """Listens on a background UDP port for remote focus commands (independent of ROS 2)."""
    import socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.bind(('0.0.0.0', port))
    except Exception as e:
        print(f"[Autofocus] WARNING: Could not bind to UDP trigger port {port}: {e}")
        return
        
    print(f"[Autofocus] Remote UDP trigger listener active on port {port}.")
    
    while True:
        try:
            data, addr = sock.recvfrom(1024)
            cmd = data.decode().strip().lower()
            if cmd in ['focus', 'autofocus', 'af']:
                print(f"[Autofocus] Remote trigger received from {addr[0]}. Starting sweep...")
                streamer.trigger_sweep()
        except Exception:
            break


def run_standalone_camera(streamer, device_id, camera_type='v4l2', autofocus_on_start=False):
    """Captures from a hardware camera index or device path, using the selected backend."""
    print(f"Starting standalone camera stream from device {device_id} using '{camera_type}' backend...")
    
    if camera_type == 'csi':
        try:
            sensor_id = int(device_id)
        except ValueError:
            sensor_id = 0
            
        gst_cap_pipeline = (
            f"nvarguscamerasrc sensor-id={sensor_id} ! "
            f"video/x-raw(memory:NVMM), width={streamer.width}, height={streamer.height}, format=NV12, framerate={streamer.fps}/1 ! "
            f"nvvidconv ! "
            f"video/x-raw, format=BGRx ! "
            f"videoconvert ! "
            f"video/x-raw, format=BGR ! "
            f"appsink drop=true"
        )
        print(f"Opening CSI camera with GStreamer pipeline:\n{gst_cap_pipeline}")
        cap = cv2.VideoCapture(gst_cap_pipeline, cv2.CAP_GSTREAMER)
        
    elif camera_type == 'gstreamer':
        try:
            dev_num = int(device_id)
            dev_path = f"/dev/video{dev_num}"
        except ValueError:
            dev_path = device_id
            
        gst_cap_pipeline = (
            f"v4l2src device={dev_path} ! "
            f"image/jpeg, width={streamer.width}, height={streamer.height}, framerate={streamer.fps}/1 ! "
            f"jpegdec ! "
            f"videoconvert ! "
            f"video/x-raw, format=BGR ! "
            f"appsink drop=true"
        )
        print(f"Opening USB camera with GStreamer pipeline:\n{gst_cap_pipeline}")
        cap = cv2.VideoCapture(gst_cap_pipeline, cv2.CAP_GSTREAMER)
        
    else:  # 'v4l2'
        try:
            device = int(device_id)
        except ValueError:
            device = device_id
            
        print(f"Opening V4L2 device {device_id} directly...")
        cap = cv2.VideoCapture(device, cv2.CAP_V4L2)
        
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, streamer.width)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, streamer.height)
        cap.set(cv2.CAP_PROP_FPS, streamer.fps)

    if not cap.isOpened():
        print(f"ERROR: Could not open camera device {device_id} with backend '{camera_type}'.")
        return

    # trigger autofocus on startup if enabled
    if autofocus_on_start:
        streamer.trigger_sweep()

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
    parser.add_argument('--camera-type', type=str, default='v4l2', choices=['v4l2', 'csi', 'gstreamer'],
                        help='Camera capture backend: "v4l2", "csi", or "gstreamer"')
    parser.add_argument('--topic', type=str, default='/image_raw', help='ROS 2 image topic to subscribe to')
    parser.add_argument('--mjpeg', action='store_true', help='Use MJPEG compression instead of H.264')
    parser.add_argument('--standalone', action='store_true', help='Force standalone camera mode even if ROS 2 is sourced')
    parser.add_argument('--autofocus-on-start', action='store_true', help='Trigger one-shot autofocus automatically at startup')
    parser.add_argument('--trigger-port', type=int, default=5005, help='UDP port to listen for remote focus triggers')

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

    # start the remote focus trigger UDP listener thread (independent of ROS 2)
    trigger_thread = threading.Thread(
        target=start_udp_trigger_listener,
        args=(streamer, parsed_args.trigger_port),
        daemon=True
    )
    trigger_thread.start()

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
        run_standalone_camera(streamer, parsed_args.device, parsed_args.camera_type, parsed_args.autofocus_on_start)


if __name__ == '__main__':
    main()

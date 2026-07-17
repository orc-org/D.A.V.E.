#!/usr/bin/env python3

import sys
import argparse
import time
import cv2
import threading
import subprocess

# try importing ros 2 dependencies
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
    
    def __init__(self, host, port, fps, width, height, bitrate, use_mjpeg, sensor_id=0):
        self.host = host
        self.port = port
        self.fps = fps
        self.width = width
        self.height = height
        self.bitrate = bitrate
        self.use_mjpeg = use_mjpeg
        self.writer = None
        
        # autofocus state variables
        self.sensor_id = sensor_id
        # csi port 0 (camera 0) maps to i2c bus 10, csi port 1 (camera 1) maps to i2c bus 9
        self.i2c_bus = 10 if sensor_id == 0 else 9
        self.i2c_address = 0x0c
        
        self.autofocus_in_progress = False
        self.last_frame_sharpness = 0.0
        self.focus_motor_initialized = False
        self.has_focus_motor = False
        
        self.init_gst_writer()

    def _try_init_bus(self, bus):
        """Attempts to initialize the VCM focusing chip on a specific I2C bus."""
        try:
            # write 0x00 to register 0x02 to initialize the ak7375 vcm chip
            res = subprocess.run(
                ["i2cset", "-y", str(bus), f"0x{self.i2c_address:02x}", "0x02", "0x00"], 
                capture_output=True, check=True
            )
            return True
        except Exception:
            return False

    def init_focus_motor(self):
        """Initializes focus VCM on its designated bus with retry delay to allow chip boot-up."""
        # retry up to 5 times with a 50ms delay between attempts to allow VCM chip to boot after camera power-on
        for attempt in range(1, 6):
            if self._try_init_bus(self.i2c_bus):
                print(f"[Autofocus] Focus VCM initialized successfully on I2C bus {self.i2c_bus} at address 0x{self.i2c_address:02x} (attempt {attempt}).")
                self.has_focus_motor = True
                return
            time.sleep(0.05)

        print(f"[Autofocus] WARNING: Could not communicate with focus VCM on designated I2C bus {self.i2c_bus}.")
        print("[Autofocus] Direct I2C focus commands will be disabled.")
        self.has_focus_motor = False

    def set_focus(self, value):
        """Writes 10-bit focus value (scaled to 12-bit register format) to VCM via direct I2C calls."""
        if not self.has_focus_motor:
            return
        try:
            # map input range (0 to 1000) to vcm 12-bit register range (0 to 4095)
            val = int(value / 1000.0 * 4095)
            val = max(0, min(4095, val))
            val <<= 4  # shift left by 4 bits as expected by ak7375/dw9714 registers
            
            high_byte = (val >> 8) & 0xFF
            low_byte = val & 0xFF
            
            # write high byte to register 0x00 and low byte to register 0x01
            subprocess.run(["i2cset", "-y", str(self.i2c_bus), f"0x{self.i2c_address:02x}", "0x00", f"0x{high_byte:02x}"], check=True, capture_output=True)
            subprocess.run(["i2cset", "-y", str(self.i2c_bus), f"0x{self.i2c_address:02x}", "0x01", f"0x{low_byte:02x}"], check=True, capture_output=True)
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
        if not self.has_focus_motor:
            print("[Autofocus] WARNING: No focus motor detected over I2C. Focus sweep aborted.")
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
        coarse_step = 80  # increased step size for faster sweeping
        
        best_focus = focus_min
        max_sharpness = 0.0
        
        # coarse sweep
        for val in range(focus_min, focus_max + 1, coarse_step):
            self.set_focus(val)
            # reduced settle time for faster sweep
            time.sleep(0.08)
            
            sharpness = self.last_frame_sharpness
            print(f"[Autofocus] Coarse Focus: {val:3d} | Sharpness: {sharpness:.2f}")
            if sharpness > max_sharpness:
                max_sharpness = sharpness
                best_focus = val
                
        # fine sweep around the coarse peak
        fine_min = max(focus_min, best_focus - 60)
        fine_max = min(focus_max, best_focus + 60)
        fine_step = 20
        
        for val in range(fine_min, fine_max + 1, fine_step):
            self.set_focus(val)
            time.sleep(0.08)
            
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
            # mjpeg pipeline
            gst_pipeline = (
                f"appsrc ! video/x-raw, format=BGR ! queue ! videoconvert ! "
                f"jpegenc quality=80 ! rtpjpegpay ! "
                f"udpsink host={self.host} port={self.port} sync=false async=false buffer-size=2097152"
            )
        else:
            # h.264 software encoding with low latency tuning
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
        
        # lazy initialization: only initialize the vcm focus motor when the camera is active and powered
        if not self.focus_motor_initialized:
            self.init_focus_motor()
            self.focus_motor_initialized = True
        
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
            
            # register ros 2 service to trigger autofocus while running
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
    
    # try binding to the requested port, or search for the next available one
    current_port = port
    bound = False
    while not bound:
        try:
            sock.bind(('0.0.0.0', current_port))
            bound = True
        except Exception:
            # port in use, try the next one
            current_port += 1
            if current_port > port + 100:  # safety ceiling
                print("[Autofocus] WARNING: Could not find any free UDP trigger ports. Trigger listener aborted.")
                return
        
    print(f"[Autofocus] Remote UDP trigger listener active on port {current_port}.")
    
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
    parser.add_argument('--width', type=str, default='1280', help='Target frame width')
    parser.add_argument('--height', type=str, default='720', help='Target frame height')
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

    # resolve sensor-id for i2c bus mapping robustly
    sensor_id = 0
    if parsed_args.device.isdigit():
        sensor_id = int(parsed_args.device)
    elif "video" in parsed_args.device:
        try:
            sensor_id = int(''.join(filter(str.isdigit, parsed_args.device)))
        except ValueError:
            sensor_id = 0

    # instantiate the core gstreamer video writer
    streamer = VideoStreamer(
        host=parsed_args.host,
        port=parsed_args.port,
        fps=parsed_args.fps,
        width=int(parsed_args.width) if isinstance(parsed_args.width, str) and parsed_args.width.isdigit() else parsed_args.width,
        height=int(parsed_args.height) if isinstance(parsed_args.height, str) and parsed_args.height.isdigit() else parsed_args.height,
        bitrate=parsed_args.bitrate,
        use_mjpeg=parsed_args.mjpeg,
        sensor_id=sensor_id
    )

    # automatically offset the trigger port based on sensor_id to prevent overlaps
    trigger_port = parsed_args.trigger_port + sensor_id

    # start the remote focus trigger udp listener thread (independent of ros 2)
    trigger_thread = threading.Thread(
        target=start_udp_trigger_listener,
        args=(streamer, trigger_port),
        daemon=True
    )
    trigger_thread.start()

    if parsed_args.source == 'topic' and not parsed_args.standalone:
        if not ROS2_AVAILABLE:
            print("ERROR: ROS 2 imports failed. Cannot run in topic mode. Use --standalone / --source camera.")
            sys.exit(1)
        
        # initialize ros 2 and run the subscriber node
        rclpy.init()
        node = VideoStreamerROS2Node(parsed_args, streamer)
        try:
            rclpy.spin(node)
        except KeyboardInterrupt:
            print("\nShutting down ros 2 node...")
        finally:
            node.destroy_node()
            rclpy.try_shutdown()
    else:
        # run standalone camera grabber
        run_standalone_camera(streamer, parsed_args.device, parsed_args.camera_type, parsed_args.autofocus_on_start)


if __name__ == '__main__':
    main()

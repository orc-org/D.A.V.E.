# Rover Video Streamer

This package implements raw UDP-based video streaming system for a ROS 2 rover. It is good for hardware like the Jetson which lacks an NVENC hardware encoder, by utilizing an optimized CPU-based software encoder (`x264enc`).

It supports two modes of operation:
1. **Camera Mode** (default): Captures video frames directly from a local V4L2 device (e.g., `/dev/video0`) using OpenCV.
2. **ROS 2 Topic Mode**: Subscribes to a local ROS 2 image topic (e.g., `/image_raw`) and streams the frames, bypassing DDS serialization for high-bandwidth network transfer.

---

## GStreamer Pipelines Explained

### 1. Sender Pipeline (Jetson Orin Nano)
The pipeline is constructed inside OpenCV `cv2.VideoWriter`:

```
appsrc ! video/x-raw, format=BGR ! queue ! videoconvert ! video/x-raw, format=I420 ! x264enc tune=zerolatency bitrate=2000 speed-preset=ultrafast key-int-max=30 threads=4 ! rtph264pay config-interval=1 aggregate-mode=zero-latency ! udpsink host=192.168.1.10 port=5000 sync=false async=false buffer-size=2097152
```

- **`appsrc`**: The entry point where OpenCV BGR frames are fed into GStreamer.
- **`video/x-raw, format=BGR`**: Informs GStreamer of the incoming OpenCV format.
- **`queue`**: Decouples the OpenCV thread from the GStreamer processing pipeline, preventing blocking.
- **`videoconvert ! video/x-raw, format=I420`**: Converts BGR frames to YUV420p, which is required by `x264enc`.
- **`x264enc`**: The CPU H.264 software encoder.
  - `tune=zerolatency`: Disables frame reordering (B-frames) and multi-frame lookahead. This drops buffering latency to 0 frames.
  - `bitrate=2000`: Sets the target bit rate to 2000 kbps (2 Mbps) for 720p 30fps. (Helps to avoid packet loss)
  - `speed-preset=ultrafast`: Uses the fastest and least CPU-intensive encoding settings.
  - `key-int-max=30`: Forces a keyframe (I-frame) at least every 30 frames (1 second). This guarantees that the receiver will recover from network packet drops within 1 second.
  - `threads=4`: Uses 4 CPU cores to speed up encoding.
- **`rtph264pay`**: Payloads raw H.264 streams into RTP packets.
  - `config-interval=1`: Periodically sends the SPS/PPS parameter headers in-band. This allows the receiver to join the stream or recover from packet loss immediately without waiting.
  - `aggregate-mode=zero-latency`: Bundles SPS/PPS/NAL units into STAP-A packets to minimize packetization overhead and latency.
- **`udpsink`**: Sends the RTP packet stream over UDP.
  - `host=192.168.1.xx` / `port=5000`: The destination IP address (base station laptop) and port.
  - `sync=false`: Tells GStreamer to stream frames as soon as they are encoded without waiting for the pipeline clock.
  - `async=false`: Disables asynchronous state changes, reducing state transition latency.
  - `buffer-size=2097152`: Configures a 2MB socket send buffer to prevent OS-level UDP packet drops during high-throughput transmission.

### 2. Receiver Pipeline (Base Station Laptop)
Run the following GStreamer CLI command on the receiver machine (`192.168.1.xx`) [You can find your IP by running hostname -I]:


```bash
gst-launch-1.0 -v udpsrc port=5000 ! application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96 ! rtpjitterbuffer latency=0 drop-on-latency=true ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! autovideosink sync=false
```

- **`udpsrc port=5000`**: Listens for UDP packets on port 5000.
- **`application/x-rtp,...`**: Binds the static RTP caps since raw UDP doesn't support session negotiation (SDP).
- **`rtpjitterbuffer latency=0 drop-on-latency=true`**:
  - `latency=0`: Tells the jitter buffer not to buffer packets. This prioritizes low latency over smooth playback.
  - `drop-on-latency=true`: Discards any packet that arrives late relative to the playback timeline.
- **`rtph264depay ! h264parse`**: Decapsulates and parses the H.264 stream.
- **`avdec_h264`**: Fast software H.264 decoder (ffmpeg/libav-based).
- **`autovideosink sync=false`**: Renders the video window. The `sync=false` property renders frames as soon as they are decoded rather than waiting for timestamps.

---

## Installation & Setup (Orin Nano)

### 1. Required GStreamer Plugins
Install the GStreamer suite on the Jetson to ensure all plugins are available:

```bash
sudo apt-get update
sudo apt-get install -y \
    gstreamer1.0-tools \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    python3-opencv
```

*Note: The `gstreamer1.0-plugins-ugly` package contains the `x264enc` plugin. `gstreamer1.0-plugins-good` contains `rtph264pay` and `udpsink`.*

### 2. Build the Package
From the root of your ROS 2 workspace:

```bash
colcon build --packages-select rover_video_streamer
source install/setup.bash
```

---

## Usage Instructions

### 1. Run the Receiver (Base Station Laptop)
Copy the `scripts` directory to your laptop, or run it directly from the repository:

* **Linux / macOS:**
  ```bash
  ./scripts/receive_stream.sh [options]
  ```
  *Common options:*
  * `-p, --port <number>`: Select UDP port (default: `5000`)
  * `-m, --mjpeg`: Expect an MJPEG stream instead of H.264
  * `-d, --diagnostics`: Show real-time frame rates and timing statistics overlay

* **Windows:**
  ```cmd
  .\scripts\receive_stream.bat [options]
  ```

### 2. Run the Sender (Jetson Orin Nano)

#### Run as a ROS 2 Node
If you have a local image topic publishing on the Jetson (e.g., `/image_raw`), run:

```bash
ros2 run rover_video_streamer video_streamer --source topic --topic /image_raw --host 192.168.1.xx --port 5000
```

### Run as a Standalone Script (What I recommed using so far)
If you want to capture directly from a V4L2 USB/CSI camera (e.g. `/dev/video0`) without running ROS 2:

```bash
ros2 run rover_video_streamer video_streamer --camera-type csi --device 0 --host 192.168.1.xx --port 5000
```

*(Alternatively, run the script directly with python3)*:
```bash
python3 src/rover_video_streamer/rover_video_streamer/video_streamer.py --camera-type csi --device 0 --host 192.168.1.xx --port 5000
```

Multiple cameras can be streamed at once by changing the device, and port settings (i.e. --device 1 ... --port 5001)

---

## Autofocus

This package supports remote-triggered autofocus for CSI camera modules (Arducam IMX519). Since the jetson kernel V4L2 focus motor driver is disabled by default, this is achieved by writing directly to the VCM focus motor chip over userspace I2C.

### Port Mapping
To allow streaming multiple cameras simultaneously, the background UDP trigger listener port is automatically offset by the physical camera index (`sensor_id`):
- **Camera 0** (`--device 0`) listens on port **`5005`** (maps to I2C bus 10)
- **Camera 1** (`--device 1`) listens on port **`5006`** (maps to I2C bus 9)

### Trigger Focus Remotely (From Base Station Laptop)
Use the `trigger_focus.py` script to request an autofocus sweep over the network:

*   **Trigger Camera 0:**
    ```bash
    python3 scripts/trigger_focus.py 192.168.1.xx
    ```
    *(defaults to port 5005)*

*   **Trigger Camera 1:**
    ```bash
    python3 scripts/trigger_focus.py 192.168.1.xx --port 5006
    ```

### 2. Standalone Local Autofocus (Jetson)
If you want to run a one-shot autofocus sweep locally on the Jetson without video streaming active, run:
```bash
python3 scripts/autofocus.py --sensor <0 or 1>
```

---


### Lossy Link Enhancements (MJPEG Alternative)
If severe packet loss is experienced, H.264 can sometimes suffer from visual artifacts due to corrupted keyframes. You can try streaming **MJPEG** frames instead, which handles packet loss per frame at the expense of higher bandwidth:

**Jetson Command:**
```bash
ros2 run rover_video_streamer video_streamer --camera-type csi --device 0 --host 192.168.1.xx --port 5000 --mjpeg
```

**Receiver Command:**
```bash
gst-launch-1.0 -v udpsrc port=5000 ! application/x-rtp,media=video,clock-rate=90000,encoding-name=JPEG,payload=26 ! rtpjpegdepay ! jpegdec ! videoconvert ! autovideosink sync=false
```


#!/bin/bash
# Custom script to launch CSI Camera 0 at 60 FPS for high-speed Morse decoding

echo "Launching CSI Camera 0 at 60 FPS..."
python3 /home/orc/D.A.V.E./ros2_ws/src/rover_video_streamer/rover_video_streamer/video_streamer.py \
    --camera-type csi \
    --device 0 \
    --host 192.168.1.87 \
    --port 5000 \
    --fps 60 \
    --width 1280 \
    --height 720

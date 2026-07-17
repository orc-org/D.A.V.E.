#!/bin/bash

# default configurations
PORT=5000
CODEC="h264"
DIAGNOSTICS=false

# help message
show_help() {
    echo "Low-Latency Video Stream Receiver Script"
    echo "========================================"
    echo "Usage: ./receive_stream.sh [options]"
    echo ""
    echo "Options:"
    echo "  -p, --port <number>     UDP port to listen on (default: 5000)"
    echo "  -m, --mjpeg             Expect an MJPEG stream (default: H.264)"
    echo "  -d, --diagnostics       Show real-time FPS and latency diagnostics overlay"
    echo "  -h, --help              Show this help message"
    echo ""
}

# Parse options
while [[ $# -gt 0 ]]; do
    case "$1" in
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        -m|--mjpeg)
            CODEC="mjpeg"
            shift 1
            ;;
        -d|--diagnostics)
            DIAGNOSTICS=true
            shift 1
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

echo "=========================================="
echo "Starting low-latency video stream receiver"
echo "Listening Port: $PORT"
echo "Expected Codec: $CODEC"
echo "Diagnostics:    $DIAGNOSTICS"
echo "=========================================="

# construct caps and decoding elements based on selection
if [ "$CODEC" = "h264" ]; then
    CAPS="application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96"
    DECODER="rtph264depay ! h264parse ! avdec_h264"
else
    CAPS="application/x-rtp,media=video,clock-rate=90000,encoding-name=JPEG,payload=26"
    DECODER="rtpjpegdepay ! jpegdec"
fi

# construct sink based on diagnostic mode
if [ "$DIAGNOSTICS" = true ]; then
    # displays timing diagnostics overlay
    SINK="fpsdisplaysink video-sink=\"autovideosink sync=false\" sync=false"
else
    # directly displays window with zero rendering delay
    SINK="autovideosink sync=false"
fi

# full GStreamer command
PIPELINE="gst-launch-1.0 -v udpsrc port=$PORT ! $CAPS ! rtpjitterbuffer latency=0 drop-on-latency=true ! $DECODER ! videoconvert ! $SINK"

echo "Executing GStreamer pipeline:"
echo "$PIPELINE"
echo ""

eval $PIPELINE

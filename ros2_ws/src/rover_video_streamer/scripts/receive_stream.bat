@echo off
set PORT=5000
set CODEC=h264
set DIAGNOSTICS=false

:parse
IF "%~1"=="" GOTO endparse
IF "%~1"=="-p" (set PORT=%~2& shift & shift & GOTO parse)
IF "%~1"=="--port" (set PORT=%~2& shift & shift & GOTO parse)
IF "%~1"=="-m" (set CODEC=mjpeg& shift & GOTO parse)
IF "%~1"=="--mjpeg" (set CODEC=mjpeg& shift & GOTO parse)
IF "%~1"=="-d" (set DIAGNOSTICS=true& shift & GOTO parse)
IF "%~1"=="--diagnostics" (set DIAGNOSTICS=true& shift & GOTO parse)
IF "%~1"=="-h" (goto help)
IF "%~1"=="--help" (goto help)
echo Unknown option: %~1
goto help

:help
echo Low-Latency Video Stream Receiver Script (Windows)
echo ==================================================
echo Usage: receive_stream.bat [options]
echo.
echo Options:
echo   -p, --port ^<number^>     UDP port to listen on (default: 5000)
echo   -m, --mjpeg             Expect an MJPEG stream (default: H.264)
echo   -d, --diagnostics       Show real-time FPS and latency diagnostics overlay
echo   -h, --help              Show this help message
exit /b 0

:endparse
echo ==========================================
echo Starting low-latency video stream receiver
echo Listening Port: %PORT%
echo Expected Codec: %CODEC%
echo Diagnostics:    %DIAGNOSTICS%
echo ==========================================

if "%CODEC%"=="h264" (
    set CAPS="application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96"
    set DECODER=rtph264depay ! h264parse ! avdec_h264
) else (
    set CAPS="application/x-rtp,media=video,clock-rate=90000,encoding-name=JPEG,payload=26"
    set DECODER=rtpjpegdepay ! jpegdec
)

if "%DIAGNOSTICS%"=="true" (
    set SINK=fpsdisplaysink video-sink="autovideosink sync=false" sync=false
) else (
    set SINK=autovideosink sync=false
)

echo Executing GStreamer command:
echo gst-launch-1.0 -v udpsrc port=%PORT% ! %CAPS% ! rtpjitterbuffer latency=0 drop-on-latency=true ! %DECODER% ! videoconvert ! %SINK%
echo.

gst-launch-1.0 -v udpsrc port=%PORT% ! %CAPS% ! rtpjitterbuffer latency=0 drop-on-latency=true ! %DECODER% ! videoconvert ! %SINK%

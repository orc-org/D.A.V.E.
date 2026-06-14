#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"
#include "image_transport/image_transport.hpp"

#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <cmath>

struct CameraStream {
    std::string name;
    bool is_csi;
    int csi_id;
    std::string usb_device;
    bool enabled;
    bool is_fallback;
    
    cv::VideoCapture cap;
    std::thread capture_thread;
    std::mutex frame_mutex;
    cv::Mat latest_frame;
    bool has_new_frame;
    bool stop_thread;
    
    image_transport::Publisher pub;

    CameraStream() : 
        is_csi(false), csi_id(0), enabled(false), 
        is_fallback(false), has_new_frame(false), stop_thread(false) {}
};

class MultiCameraNode : public rclcpp::Node
{
public:
    MultiCameraNode() : Node("multi_camera_node")
    {
        this->declare_parameter<bool>("csi_0_enable", true);
        this->declare_parameter<bool>("csi_1_enable", true);
        this->declare_parameter<bool>("usb_enable", true);
        this->declare_parameter<std::string>("usb_device", "/dev/video2");
        this->declare_parameter<int>("width", 1280);
        this->declare_parameter<int>("height", 720);
        this->declare_parameter<int>("fps", 30);

        bool csi_0_enable = this->get_parameter("csi_0_enable").as_bool();
        bool csi_1_enable = this->get_parameter("csi_1_enable").as_bool();
        bool usb_enable = this->get_parameter("usb_enable").as_bool();
        std::string usb_device = this->get_parameter("usb_device").as_string();
        int width = this->get_parameter("width").as_int();
        int height = this->get_parameter("height").as_int();
        int fps = this->get_parameter("fps").as_int();

        RCLCPP_INFO(this->get_logger(), "Initializing Multi-Camera Node (720p @ 30fps)...");

        // setup image transport
        it_ = std::make_shared<image_transport::ImageTransport>(shared_from_this());

        // initialize camera streams
        for (int i = 0; i < 3; ++i) {
            streams_.push_back(std::make_unique<CameraStream>());
        }
        
        // CSI camera 0
        streams_[0]->name = "csi_0";
        streams_[0]->is_csi = true;
        streams_[0]->csi_id = 0;
        streams_[0]->enabled = csi_0_enable;

        // CSI camera 1
        streams_[1]->name = "csi_1";
        streams_[1]->is_csi = true;
        streams_[1]->csi_id = 1;
        streams_[1]->enabled = csi_1_enable;

        // USB camera
        streams_[2]->name = "usb_back";
        streams_[2]->is_csi = false;
        streams_[2]->usb_device = usb_device;
        streams_[2]->enabled = usb_enable;

        // open Cameras & launch threads
        for (auto& stream : streams_) {
            if (!stream->enabled) {
                RCLCPP_INFO(this->get_logger(), "Stream [%s] is disabled.", stream->name.c_str());
                continue;
            }

            stream->pub = it_->advertise("/camera/" + stream->name + "/image_raw", 10);
            
            bool success = false;
            if (stream->is_csi) {
                std::string pipeline = get_csi_pipeline(stream->csi_id, width, height, fps);
                RCLCPP_INFO(this->get_logger(), "Opening CSI Sensor [%d] via GStreamer...", stream->csi_id);
                success = stream->cap.open(pipeline, cv::CAP_GSTREAMER);
            } else {
                int index = get_device_index(stream->usb_device);
                RCLCPP_INFO(this->get_logger(), "Opening USB Device [%s] (Index %d)...", stream->usb_device.c_str(), index);
                success = stream->cap.open(index, cv::CAP_V4L2);
                if (success) {
                    stream->cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
                    stream->cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
                    stream->cap.set(cv::CAP_PROP_FPS, fps);
                }
            }

            if (!success) {
                RCLCPP_WARN(this->get_logger(), 
                    "Failed to open hardware stream [%s]. Falling back to test pattern generator.", 
                    stream->name.c_str());
                stream->is_fallback = true;
            } else {
                RCLCPP_INFO(this->get_logger(), "Successfully opened hardware stream [%s].", stream->name.c_str());
            }

            // start thread-safe capture worker
            stream->stop_thread = false;
            stream->capture_thread = std::thread(&MultiCameraNode::capture_loop, this, stream.get());
        }

        // publisher timer at 30Hz
        double timer_period_ms = 1000.0 / static_cast<double>(fps);
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(timer_period_ms)),
            std::bind(&MultiCameraNode::publish_loop, this)
        );

        RCLCPP_INFO(this->get_logger(), "Multi-Camera Streaming Pipeline Started.");
    }

    ~MultiCameraNode() override
    {
        RCLCPP_INFO(this->get_logger(), "Stopping capture threads and releasing cameras...");
        for (auto& stream : streams_) {
            stream->stop_thread = true;
            if (stream->capture_thread.joinable()) {
                stream->capture_thread.join();
            }
            if (stream->cap.isOpened()) {
                stream->cap.release();
            }
        }
        RCLCPP_INFO(this->get_logger(), "Multi-Camera Node Clean Exit.");
    }

private:
    std::string get_csi_pipeline(int sensor_id, int width, int height, int fps) {
        return "nvarguscamerasrc sensor-id=" + std::to_string(sensor_id) +
               " ! video/x-raw(memory:NVMM), width=" + std::to_string(width) +
               ", height=" + std::to_string(height) +
               ", format=NV12, framerate=" + std::to_string(fps) + "/1" +
               " ! nvvidconv ! video/x-raw, format=BGRx" +
               " ! videoconvert ! video/x-raw, format=BGR ! appsink drop=true sync=false";
    }

    int get_device_index(const std::string& path) {
        // find last numeric string in path (e.g. /dev/video2 -> 2)
        size_t last_num_idx = path.find_last_of("0123456789");
        if (last_num_idx != std::string::npos) {
            size_t start_idx = last_num_idx;
            while (start_idx > 0 && isdigit(path[start_idx - 1])) {
                start_idx--;
            }
            std::string num_str = path.substr(start_idx, last_num_idx - start_idx + 1);
            try {
                return std::stoi(num_str);
            } catch (...) {}
        }
        return 0; // default to first USB capture card
    }

    void capture_loop(CameraStream* stream) {
        double angle = 0.0;
        // SLOP WARNING D:
        while (rclcpp::ok() && !stream->stop_thread) {
            if (stream->is_fallback) {
                // generate a highly visual animated test pattern
                cv::Mat frame = cv::Mat::zeros(720, 1280, CV_8UC3);
                
                // draw a retro sci-fi background pattern
                for (int r = 0; r < frame.rows; r += 10) {
                    cv::line(frame, cv::Point(0, r), cv::Point(frame.cols, r), 
                             cv::Scalar(15, 25, 15), 1);
                }
                for (int c = 0; c < frame.cols; c += 20) {
                    cv::line(frame, cv::Point(c, 0), cv::Point(c, frame.rows), 
                             cv::Scalar(15, 25, 15), 1);
                }

                // draw moving target crosshairs
                angle += 0.08;
                int cx = 640 + 350 * cos(angle);
                int cy = 360 + 200 * sin(angle);
                
                // outer circle
                cv::circle(frame, cv::Point(cx, cy), 50, cv::Scalar(0, 180, 255), 2);
                // inner solid core
                cv::circle(frame, cv::Point(cx, cy), 8, cv::Scalar(0, 255, 128), -1);
                // crosshair tick lines
                cv::line(frame, cv::Point(cx - 70, cy), cv::Point(cx + 70, cy), cv::Scalar(0, 180, 255), 1);
                cv::line(frame, cv::Point(cx, cy - 70), cv::Point(cx, cy + 70), cv::Scalar(0, 180, 255), 1);

                // overlay text header
                std::string header = stream->name + " Stream [OFFLINE SIM]";
                cv::putText(frame, header, cv::Point(50, 80), 
                            cv::FONT_HERSHEY_SIMPLEX, 1.4, cv::Scalar(0, 180, 255), 3);

                // add simulated telemetry info
                char telemetry_buf[128];
                snprintf(telemetry_buf, sizeof(telemetry_buf), "RES: 1280x720 | FPS: 30.0 | STATUS: Fallback Simulated");
                cv::putText(frame, telemetry_buf, cv::Point(50, 130), 
                            cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(150, 180, 200), 2);

                // overlay time
                time_t now = time(nullptr);
                struct tm* tstruct = localtime(&now);
                char time_buf[80];
                strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %X", tstruct);
                cv::putText(frame, time_buf, cv::Point(50, 660), 
                            cv::FONT_HERSHEY_SIMPLEX, 1.1, cv::Scalar(0, 255, 128), 2);

                {
                    std::lock_guard<std::mutex> lock(stream->frame_mutex);
                    stream->latest_frame = frame;
                    stream->has_new_frame = true;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 fps loop
            } else {
                cv::Mat frame;
                if (stream->cap.read(frame)) {
                    if (!frame.empty()) {
                        std::lock_guard<std::mutex> lock(stream->frame_mutex);
                        stream->latest_frame = frame;
                        stream->has_new_frame = true;
                    }
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
        }
    }

    void publish_loop() {
        auto now = this->get_clock()->now();
        
        for (auto& stream : streams_) {
            if (!stream->enabled) continue;
            
            cv::Mat frame_to_publish;
            bool pub_ready = false;
            
            {
                std::lock_guard<std::mutex> lock(stream->frame_mutex);
                if (stream->has_new_frame && !stream->latest_frame.empty()) {
                    frame_to_publish = stream->latest_frame.clone();
                    stream->has_new_frame = false;
                    pub_ready = true;
                }
            }
            
            if (pub_ready) {
                auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame_to_publish).toImageMsg();
                msg->header.stamp = now;
                msg->header.frame_id = stream->name + "_frame";
                stream->pub.publish(*msg);
            }
        }
    }

    std::shared_ptr<image_transport::ImageTransport> it_;
    std::vector<std::unique_ptr<CameraStream>> streams_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MultiCameraNode>());
    rclcpp::shutdown();
    return 0;
}

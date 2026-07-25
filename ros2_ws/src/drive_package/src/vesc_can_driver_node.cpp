#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <fcntl.h>

class VescCanDriverNode : public rclcpp::Node
{
public:
    VescCanDriverNode() : Node("vesc_can_driver_node"), can_socket_(-1)
    {
        // declare and retrieve VESC driver parameters
        this->declare_parameter<std::string>("can_interface", "can0");
        this->declare_parameter<std::string>("control_mode", "duty"); // "duty", "erpm", "current"
        this->declare_parameter<double>("max_erpm", 15000.0);
        this->declare_parameter<double>("max_current", 30.0); // max current in amps
        this->declare_parameter<double>("max_duty", 0.35); // max duty cycle limit (0.0 to 1.0)
        this->declare_parameter<double>("ramp_rate", 2.0); // max velocity change per second (e.g. 2.0 = 0.5s for 0->1.0)
        this->declare_parameter<double>("publish_rate", 20.0); // CAN transmit loop frequency in Hz
        
        // CAN IDs for the 4 wheels
        this->declare_parameter<int>("fl_can_id", 53);
        this->declare_parameter<int>("fr_can_id", 44);
        this->declare_parameter<int>("rl_can_id", -1);
        this->declare_parameter<int>("rr_can_id", -1);

        can_interface_ = this->get_parameter("can_interface").as_string();
        control_mode_ = this->get_parameter("control_mode").as_string();
        max_erpm_ = this->get_parameter("max_erpm").as_double();
        max_current_ = this->get_parameter("max_current").as_double();
        max_duty_ = this->get_parameter("max_duty").as_double();
        ramp_rate_ = this->get_parameter("ramp_rate").as_double();
        publish_rate_ = this->get_parameter("publish_rate").as_double();

        fl_can_id_ = this->get_parameter("fl_can_id").as_int();
        fr_can_id_ = this->get_parameter("fr_can_id").as_int();
        rl_can_id_ = this->get_parameter("rl_can_id").as_int();
        rr_can_id_ = this->get_parameter("rr_can_id").as_int();

        // initialize SocketCAN interface
        if (initCanSocket(can_interface_))
        {
            RCLCPP_INFO(this->get_logger(), "Successfully initialized CAN socket on %s", can_interface_.c_str());
        }
        else
        {
            RCLCPP_WARN(this->get_logger(), "CAN socket initialization failed. Running in simulation mode.");
        }

        // subscriptions to independent motor command channels
        rclcpp::QoS sub_qos(10);
        sub_qos.reliable();
        sub_qos.durability_volatile();

        fl_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/front_left", sub_qos,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                RCLCPP_INFO(this->get_logger(), "Received front_left: %.3f", msg->data);
                target_fl_ = std::max(-1.0f, std::min(1.0f, msg->data));
            });

        fr_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/front_right", sub_qos,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                target_fr_ = std::max(-1.0f, std::min(1.0f, msg->data));
            });

        rl_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/rear_left", sub_qos,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                target_rl_ = std::max(-1.0f, std::min(1.0f, msg->data));
            });

        rr_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/rear_right", sub_qos,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                target_rr_ = std::max(-1.0f, std::min(1.0f, msg->data));
            });

        // periodic CAN transmit & ramping loop
        double timer_sec = 1.0 / publish_rate_;
        timer_ = this->create_wall_timer(
            std::chrono::duration<double>(timer_sec),
            std::bind(&VescCanDriverNode::updateLoop, this));

        RCLCPP_INFO(this->get_logger(), "VESC CAN Driver Node Initialized.");
        RCLCPP_INFO(this->get_logger(), "Interface: %s | Mode: %s | Max Duty: %.2f | Ramp Rate: %.1f/s", 
                    can_interface_.c_str(), control_mode_.c_str(), max_duty_, ramp_rate_);
    }

    ~VescCanDriverNode()
    {
        if (can_socket_ != -1)
        {
            close(can_socket_);
            RCLCPP_INFO(this->get_logger(), "Closed CAN socket.");
        }
    }

private:
    bool initCanSocket(const std::string &interface)
    {
        can_socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (can_socket_ < 0)
        {
            RCLCPP_ERROR(this->get_logger(), "Socket creation failed for CAN: %s", std::strerror(errno));
            return false;
        }

        struct ifreq ifr;
        std::strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';

        if (ioctl(can_socket_, SIOCGIFINDEX, &ifr) < 0)
        {
            RCLCPP_ERROR(this->get_logger(), "Failed to get CAN interface index for %s: %s", interface.c_str(), std::strerror(errno));
            close(can_socket_);
            can_socket_ = -1;
            return false;
        }

        struct sockaddr_can addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        if (bind(can_socket_, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            RCLCPP_ERROR(this->get_logger(), "Binding CAN socket failed: %s", std::strerror(errno));
            close(can_socket_);
            can_socket_ = -1;
            return false;
        }

        // make socket non-blocking to prevent locking up the ROS 2 executor when the TX queue is full
        int flags = fcntl(can_socket_, F_GETFL, 0);
        fcntl(can_socket_, F_SETFL, flags | O_NONBLOCK);

        return true;
    }

    void sendVescCanFrame(int can_id, uint8_t comm_id, int32_t val)
    {
        if (can_socket_ == -1 || can_id < 0)
        {
            return;
        }

        struct can_frame frame;
        uint32_t raw_id = (static_cast<uint32_t>(comm_id) << 8) | (static_cast<uint32_t>(can_id) & 0xFF);
        
        frame.can_id = raw_id | CAN_EFF_FLAG;
        frame.can_dlc = 4;

        frame.data[0] = (val >> 24) & 0xFF;
        frame.data[1] = (val >> 16) & 0xFF;
        frame.data[2] = (val >> 8) & 0xFF;
        frame.data[3] = val & 0xFF;

        ssize_t bytes_written = write(can_socket_, &frame, sizeof(struct can_frame));
        if (bytes_written < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK) 
            {
                RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                                     "Failed to write to CAN socket: %s", std::strerror(errno));
            }
        }
    }

    void updateLoop()
    {
        double dt = 1.0 / publish_rate_;
        double max_change = ramp_rate_ * dt;

        // apply ramping to each motor
        current_fl_ = rampValue(current_fl_, target_fl_, max_change);
        current_fr_ = rampValue(current_fr_, target_fr_, max_change);
        current_rl_ = rampValue(current_rl_, target_rl_, max_change);
        current_rr_ = rampValue(current_rr_, target_rr_, max_change);

        // transmit updated setpoints to active CAN IDs
        processAndSend("Front Left", current_fl_, fl_can_id_);
        processAndSend("Front Right", current_fr_, fr_can_id_);
        processAndSend("Rear Left", current_rl_, rl_can_id_);
        processAndSend("Rear Right", current_rr_, rr_can_id_);
    }

    float rampValue(float current, float target, double max_change)
    {
        if (current < target)
        {
            return std::min(target, static_cast<float>(current + max_change));
        }
        else if (current > target)
        {
            return std::max(target, static_cast<float>(current - max_change));
        }
        return current;
    }

    void processAndSend(const std::string &wheel_name, float velocity, int can_id)
    {
        if (can_id < 0) return;

        uint8_t comm_id = 0; 
        int32_t send_val = 0;

        if (control_mode_ == "erpm")
        {
            comm_id = 3; // CAN_PACKET_SET_RPM
            send_val = static_cast<int32_t>(velocity * max_erpm_);
        }
        else if (control_mode_ == "current")
        {
            comm_id = 1; // CAN_PACKET_SET_CURRENT
            send_val = static_cast<int32_t>(velocity * max_current_ * 1000.0);
        }
        else // default to duty cycle
        {
            comm_id = 0; // CAN_PACKET_SET_DUTY
            float scaled_velocity = velocity * static_cast<float>(max_duty_);
            send_val = static_cast<int32_t>(scaled_velocity * 100000.0f);
            
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                "[%s] CAN ID=%d | cmd=%.3f | scaled=%.3f | send_val=%d",
                wheel_name.c_str(), can_id, velocity, scaled_velocity, send_val);
        }

        sendVescCanFrame(can_id, comm_id, send_val);
    }

    // parameters
    std::string can_interface_;
    std::string control_mode_;
    double max_erpm_;
    double max_current_;
    double max_duty_;
    double ramp_rate_;
    double publish_rate_;

    int fl_can_id_;
    int fr_can_id_;
    int rl_can_id_;
    int rr_can_id_;

    // target inputs from topics
    float target_fl_ = 0.0f;
    float target_fr_ = 0.0f;
    float target_rl_ = 0.0f;
    float target_rr_ = 0.0f;

    // current ramped outputs sent to CAN
    float current_fl_ = 0.0f;
    float current_fr_ = 0.0f;
    float current_rl_ = 0.0f;
    float current_rr_ = 0.0f;

    int can_socket_;

    // subscriptions and timer
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr fl_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr fr_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr rl_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr rr_sub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VescCanDriverNode>());
    rclcpp::shutdown();
    return 0;
}

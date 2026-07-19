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
        
        // CAN IDs for the 4 wheels
        this->declare_parameter<int>("fl_can_id", 1);
        this->declare_parameter<int>("fr_can_id", 2);
        this->declare_parameter<int>("rl_can_id", 3);
        this->declare_parameter<int>("rr_can_id", 4);

        can_interface_ = this->get_parameter("can_interface").as_string();
        control_mode_ = this->get_parameter("control_mode").as_string();
        max_erpm_ = this->get_parameter("max_erpm").as_double();
        max_current_ = this->get_parameter("max_current").as_double();

        fl_can_id_ = this->get_parameter("fl_can_id").as_int();
        fr_can_id_ = this->get_parameter("fr_can_id").as_int();
        rl_can_id_ = this->get_parameter("rl_can_id").as_int();
        rr_can_id_ = this->get_parameter("rr_can_id").as_int();

        // Initialize SocketCAN interface
        if (initCanSocket(can_interface_))
        {
            RCLCPP_INFO(this->get_logger(), "Successfully initialized CAN socket on %s", can_interface_.c_str());
        }
        else
        {
            RCLCPP_WARN(this->get_logger(), "CAN socket initialization failed. Running in simulation mode.");
        }

        // Subscriptions to independent motor command channels
        fl_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/front_left", 10,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                processMotorCommand("Front Left", msg->data, fl_can_id_);
            });

        fr_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/front_right", 10,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                processMotorCommand("Front Right", msg->data, fr_can_id_);
            });

        rl_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/rear_left", 10,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                processMotorCommand("Rear Left", msg->data, rl_can_id_);
            });

        rr_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/rear_right", 10,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                processMotorCommand("Rear Right", msg->data, rr_can_id_);
            });

        RCLCPP_INFO(this->get_logger(), "VESC CAN Driver Node Initialized.");
        RCLCPP_INFO(this->get_logger(), "Interface: %s | Mode: %s | Max ERPM: %.1f | Max Current: %.1f A", 
                    can_interface_.c_str(), control_mode_.c_str(), max_erpm_, max_current_);
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

        return true;
    }

    void sendVescCanFrame(int can_id, uint8_t comm_id, int32_t val)
    {
        if (can_socket_ == -1)
        {
            RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "CAN socket offline, ignoring command transmission.");
            return;
        }

        struct can_frame frame;
        
        // VESC CAN ID configuration:
        // Bits 0-7: Controller ID (can_id)
        // Bits 8-15: Command ID (comm_id)
        // Bits 16-28: Host/Master ID (0)
        uint32_t raw_id = (static_cast<uint32_t>(comm_id) << 8) | (static_cast<uint32_t>(can_id) & 0xFF);
        
        // Use 29-bit Extended Frame Format (EFF)
        frame.can_id = raw_id | CAN_EFF_FLAG;
        frame.can_dlc = 4; // VESC motor setpoint commands send a 4-byte int

        frame.data[0] = (val >> 24) & 0xFF;
        frame.data[1] = (val >> 16) & 0xFF;
        frame.data[2] = (val >> 8) & 0xFF;
        frame.data[3] = val & 0xFF;

        ssize_t bytes_written = write(can_socket_, &frame, sizeof(struct can_frame));
        if (bytes_written < 0)
        {
            RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                                 "Failed to write to CAN socket: %s", std::strerror(errno));
        }
    }

    void processMotorCommand(const std::string &wheel_name, float cmd, int can_id)
    {
        float velocity = std::max(-1.0f, std::min(1.0f, cmd));
        
        // VESC CAN commands:
        // CAN_PACKET_SET_DUTY = 0
        // CAN_PACKET_SET_CURRENT = 1
        // CAN_PACKET_SET_RPM = 3
        uint8_t comm_id = 0; 
        int32_t send_val = 0;

        if (control_mode_ == "erpm")
        {
            comm_id = 3; // CAN_PACKET_SET_RPM
            send_val = static_cast<int32_t>(velocity * max_erpm_);
            
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                "[%s Wheel] -> Native CAN: ID=%d | Comm=SET_RPM | Val=%d",
                wheel_name.c_str(), can_id, send_val);
        }
        else if (control_mode_ == "current")
        {
            comm_id = 1; // CAN_PACKET_SET_CURRENT
            send_val = static_cast<int32_t>(velocity * max_current_ * 1000.0);
            
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                "[%s Wheel] -> Native CAN: ID=%d | Comm=SET_CURRENT | Val=%.2f A",
                wheel_name.c_str(), can_id, (static_cast<float>(send_val) / 1000.0f));
        }
        else // default to duty cycle
        {
            comm_id = 0; // CAN_PACKET_SET_DUTY
            send_val = static_cast<int32_t>(velocity * 100000.0f);
            
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                "[%s Wheel] -> Native CAN: ID=%d | Comm=SET_DUTY | Val=%.3f",
                wheel_name.c_str(), can_id, (static_cast<float>(send_val) / 100000.0f));
        }

        sendVescCanFrame(can_id, comm_id, send_val);
    }

    // Parameters
    std::string can_interface_;
    std::string control_mode_;
    double max_erpm_;
    double max_current_;

    int fl_can_id_;
    int fr_can_id_;
    int rl_can_id_;
    int rr_can_id_;

    int can_socket_;

    // subscriptions
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr fl_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr fr_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr rl_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr rr_sub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VescCanDriverNode>());
    rclcpp::shutdown();
    return 0;
}

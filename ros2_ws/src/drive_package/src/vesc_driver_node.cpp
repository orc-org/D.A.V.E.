#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

class VescDriverNode : public rclcpp::Node
{
public:
    VescDriverNode() : Node("vesc_driver_node"), serial_fd_(-1)
    {
        // declare and retrieve VESC driver parameters, change all of these later
        this->declare_parameter<std::string>("vesc_port", "/dev/ttyACM0");
        this->declare_parameter<int>("vesc_baud", 115200);
        this->declare_parameter<std::string>("control_mode", "duty"); // "duty", "erpm", "current"
        this->declare_parameter<double>("max_erpm", 15000.0);
        this->declare_parameter<double>("max_current", 30.0); // max current in amps
        
        // CAN ID configurations for 4 wheels (-1 means direct serial control without CAN forwarding)
        this->declare_parameter<int>("fl_can_id", 1);
        this->declare_parameter<int>("fr_can_id", 2);
        this->declare_parameter<int>("rl_can_id", 3);
        this->declare_parameter<int>("rr_can_id", 4);

        vesc_port_ = this->get_parameter("vesc_port").as_string();
        vesc_baud_ = this->get_parameter("vesc_baud").as_int();
        control_mode_ = this->get_parameter("control_mode").as_string();
        max_erpm_ = this->get_parameter("max_erpm").as_double();
        max_current_ = this->get_parameter("max_current").as_double();

        fl_can_id_ = this->get_parameter("fl_can_id").as_int();
        fr_can_id_ = this->get_parameter("fr_can_id").as_int();
        rl_can_id_ = this->get_parameter("rl_can_id").as_int();
        rr_can_id_ = this->get_parameter("rr_can_id").as_int();

        // attempt to open serial port
        openSerial(vesc_port_, vesc_baud_);

        // subscriptions to independent motor command channels
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

        RCLCPP_INFO(this->get_logger(), "VESC BLDC Driver Node Initialized.");
        RCLCPP_INFO(this->get_logger(), "Mode: %s | Max ERPM: %.1f | Max Current: %.1f A", 
                    control_mode_.c_str(), max_erpm_, max_current_);
    }

    ~VescDriverNode()
    {
        if (serial_fd_ != -1)
        {
            close(serial_fd_);
        }
    }

private:
    // CRC-16 CCITT implementation for VESC protocol validation
    unsigned short crc16(const unsigned char *buf, unsigned int len)
    {
        unsigned short crc = 0;
        for (unsigned int i = 0; i < len; i++)
        {
            crc ^= (unsigned short)buf[i] << 8;
            for (int j = 0; j < 8; j++)
            {
                if (crc & 0x8000)
                {
                    crc = (crc << 1) ^ 0x1021;
                }
                else
                {
                    crc <<= 1;
                }
            }
        }
        return crc;
    }

    bool openSerial(const std::string &port, int baud)
    {
        serial_fd_ = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (serial_fd_ == -1)
        {
            RCLCPP_WARN(this->get_logger(), "Failed to open serial port: %s. Running in Simulation/Offline mode.", port.c_str());
            return false;
        }

        struct termios options;
        tcgetattr(serial_fd_, &options);

        speed_t speed = B115200;
        if (baud == 9600) speed = B9600;
        else if (baud == 19200) speed = B19200;
        else if (baud == 38400) speed = B38400;
        else if (baud == 57600) speed = B57600;
        else if (baud == 115200) speed = B115200;

        cfsetispeed(&options, speed);
        cfsetospeed(&options, speed);

        options.c_cflag |= (CLOCAL | CREAD);
        options.c_cflag &= ~PARENB; // No parity
        options.c_cflag &= ~CSTOPB; // 1 stop bit
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;     // 8 data bits

        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
        options.c_oflag &= ~OPOST; // Raw output

        tcsetattr(serial_fd_, TCSANOW, &options);
        RCLCPP_INFO(this->get_logger(), "Successfully opened VESC serial port: %s at %d baud", port.c_str(), baud);
        return true;
    }

    void writeSerial(const unsigned char *data, size_t len)
    {
        if (serial_fd_ == -1) return;
        ssize_t bytes_written = write(serial_fd_, data, len);
        if (bytes_written < 0)
        {
            RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "Failed to write to VESC serial port");
        }
    }

    void sendVescCommand(int can_id, unsigned char comm_id, int32_t val)
    {
        std::vector<unsigned char> payload;
        
        // CAN Forwarding protocol header
        if (can_id >= 0)
        {
            payload.push_back(34); // COMM_FORWARD_CAN
            payload.push_back(static_cast<unsigned char>(can_id));
        }
        
        payload.push_back(comm_id);
        payload.push_back((val >> 24) & 0xFF);
        payload.push_back((val >> 16) & 0xFF);
        payload.push_back((val >> 8) & 0xFF);
        payload.push_back(val & 0xFF);

        std::vector<unsigned char> packet;
        int len = payload.size();
        if (len <= 256)
        {
            packet.push_back(2); // Start byte
            packet.push_back(static_cast<unsigned char>(len));
        }
        else
        {
            packet.push_back(3); // Start byte (large packet)
            packet.push_back((len >> 8) & 0xFF);
            packet.push_back(len & 0xFF);
        }

        packet.insert(packet.end(), payload.begin(), payload.end());

        unsigned short crc = crc16(payload.data(), len);
        packet.push_back((crc >> 8) & 0xFF);
        packet.push_back(crc & 0xFF);

        packet.push_back(3); // End byte

        writeSerial(packet.data(), packet.size());
    }

    void processMotorCommand(const std::string &wheel_name, float cmd, int can_id)
    {
        float velocity = std::max(-1.0f, std::min(1.0f, cmd));
        unsigned char comm_id = 5; // COMM_SET_DUTY
        int32_t send_val = 0;

        if (control_mode_ == "erpm")
        {
            comm_id = 8; // COMM_SET_ERPM
            send_val = static_cast<int32_t>(velocity * max_erpm_);
            
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                "[%s Wheel] -> VESC Conversion: cmd=%.2f | ERPM=%d | CAN ID=%d",
                wheel_name.c_str(), velocity, send_val, can_id);
        }
        else if (control_mode_ == "current")
        {
            comm_id = 6; // COMM_SET_CURRENT
            send_val = static_cast<int32_t>(velocity * max_current_ * 1000.0);
            
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                "[%s Wheel] -> VESC Conversion: cmd=%.2f | Current=%.2f A | CAN ID=%d",
                wheel_name.c_str(), velocity, (static_cast<float>(send_val) / 1000.0f), can_id);
        }
        else // default to duty cycle
        {
            comm_id = 5; // COMM_SET_DUTY
            send_val = static_cast<int32_t>(velocity * 100000.0f);
            
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                "[%s Wheel] -> VESC Conversion: cmd=%.2f | Duty=%.3f | CAN ID=%d",
                wheel_name.c_str(), velocity, (static_cast<float>(send_val) / 100000.0f), can_id);
        }

        sendVescCommand(can_id, comm_id, send_val);
    }

    // Parameters
    std::string vesc_port_;
    int vesc_baud_;
    std::string control_mode_;
    double max_erpm_;
    double max_current_;

    int fl_can_id_;
    int fr_can_id_;
    int rl_can_id_;
    int rr_can_id_;

    int serial_fd_;

    // subscriptions
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr fl_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr fr_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr rl_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr rr_sub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VescDriverNode>());
    rclcpp::shutdown();
    return 0;
}

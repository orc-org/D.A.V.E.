#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include "SerialPort.hpp"
#include "serial_interfaces/srv/converse.hpp"
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

class MorseBridgeNode : public rclcpp::Node
{
public:
    MorseBridgeNode() : Node("morse_bridge_node")
    {
        // Parameter for serial port
        this->declare_parameter("port", "/dev/ttyUSB0");
        this->declare_parameter("baud_rate", 115200);

        std::string port = this->get_parameter("port").as_string();
        int baud_rate = this->get_parameter("baud_rate").as_int();

        RCLCPP_INFO(this->get_logger(), "Connecting to %s at %d baud", port.c_str(), baud_rate);

        serial_port_ = std::make_shared<SerialPort>(port, baud_rate);
        serial_port_->begin();

        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "/morse_command", 10, std::bind(&MorseBridgeNode::topic_callback, this, std::placeholders::_1));

        converseServer = this->create_service<serial_interfaces::srv::Converse>(
            "morse_converse", 
            std::bind(&MorseBridgeNode::converseWithArduino, this, std::placeholders::_1, std::placeholders::_2));

        RCLCPP_INFO(this->get_logger(), "Morse Bridge Node initialized. Listening on /morse_command...");
    }

private:
    void topic_callback(const std_msgs::msg::String::SharedPtr msg) const
    {
        std::string command = msg->data + "\n";
        RCLCPP_INFO(this->get_logger(), "Sending morse command: '%s'", msg->data.c_str());
        serial_port_->write(command);
    }

    void converseWithArduino(
        const std::shared_ptr<serial_interfaces::srv::Converse::Request> request,
        std::shared_ptr<serial_interfaces::srv::Converse::Response> response)
    {
        std::string command = request->outgoing + "\n";
        RCLCPP_INFO(this->get_logger(), "Converse request: '%s'", command.c_str());
        serial_port_->write(command);
        std::this_thread::sleep_for(1500ms);
        response->incoming = serial_port_->read();
        RCLCPP_INFO(this->get_logger(), "Converse reply: '%s'", response->incoming.c_str());
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    rclcpp::Service<serial_interfaces::srv::Converse>::SharedPtr converseServer;
    std::shared_ptr<SerialPort> serial_port_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MorseBridgeNode>());
    rclcpp::shutdown();
    return 0;
}

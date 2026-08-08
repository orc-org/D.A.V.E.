#include <cstdio>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "SerialPort.hpp"
#include "serial_interfaces/srv/converse.hpp"
#include "serial_interfaces/srv/receive.hpp"
#include "serial_interfaces/srv/transmit.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

using std::placeholders::_1;
using std::placeholders::_2;

class XLRnode : public rclcpp::Node {

    rclcpp::Service<serial_interfaces::srv::Converse>::SharedPtr  converseServer;
    rclcpp::Service<serial_interfaces::srv::Receive>::SharedPtr   receiveServer;
    rclcpp::Service<serial_interfaces::srv::Transmit>::SharedPtr  transmitServer;

    SerialPort* xlr;

public:

    XLRnode() : Node("XLRnode") {
        this->declare_parameter("port", "/dev/ttyTHS1");
        this->declare_parameter("baud", 115200);
        
        std::string port = this->get_parameter("port").as_string();
        int baud = this->get_parameter("baud").as_int();
        
        xlr = new SerialPort(SerialPort::stringToCharacterArray(port), baud);

        converseServer = this->create_service<serial_interfaces::srv::Converse>("Converse", std::bind(&XLRnode::converseWithTerminal, this, _1, _2));
        receiveServer = this->create_service<serial_interfaces::srv::Receive>("Receive", std::bind(&XLRnode::receiveMessage, this, _1, _2));
        transmitServer = this->create_service<serial_interfaces::srv::Transmit>("Transmit", std::bind(&XLRnode::transmitMessage, this, _1, _2));
    }

    void transmitMessage(const std::shared_ptr<serial_interfaces::srv::Transmit::Request> request, std::shared_ptr<serial_interfaces::srv::Transmit::Response> response){
        if (!xlr->portIsOpen)
            xlr->begin();

        xlr->write(request->outgoing);
    }

    void receiveMessage(const std::shared_ptr<serial_interfaces::srv::Receive::Request> request, std::shared_ptr<serial_interfaces::srv::Receive::Response> response){
        if(!xlr->portIsOpen)
            xlr->begin();
        
        response->incoming = xlr->read();
        std::cout << response->incoming << std::endl;
    }

    void converseWithTerminal(const std::shared_ptr<serial_interfaces::srv::Converse::Request> request, std::shared_ptr<serial_interfaces::srv::Converse::Response> response){
        if(!xlr->portIsOpen)
            xlr->begin();                      // ensure port is open
        xlr->write(request->outgoing);         // send message
        std::this_thread::sleep_for(1500ms);   // wait 1.5s to allow the terminal to think it over
        response->incoming = xlr->read();      // get response
        std::cout << response->incoming << std::endl;    // print response
    }
    
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<XLRnode>());
    rclcpp::shutdown();

    return 0;
}
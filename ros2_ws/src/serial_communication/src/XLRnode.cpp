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

class XLRnode : public rclcpp::Node {

    rclcpp::Service<serial_interfaces::srv::Converse>::SharedPtr  converseServer;
    rclcpp::Service<serial_interfaces::srv::Receive>::SharedPtr   receiveServer;
    rclcpp::Service<serial_interfaces::srv::Transmit>::SharedPtr  transmitServer;

    SerialPort* xlr;
    bool portIsOpen;

public:

    XLRnode() : Node("XLRnode") {
        xlr = new SerialPort(SerialPort::stringToCharacterArray("/dev/ttyUSB0"), 115200);
        portIsOpen = false;

        converseServer = this->create_service<serial_interfaces::srv::Converse>("Converse", &converseWithTerminal);
        receiveServer = this->create_service<serial_interfaces::srv::Receive>("Receive", &receiveMessage);
        transmitServer = this->create_service<serial_interfaces::srv::Transmit>("Transmit", &transmitMessage);
    }

    void transmitMessage(const std::shared_ptr<serial_interfaces::srv::Transmit::Request> request){
        if (!portIsOpen)
            xlr->begin();

        xlr->write(request->outgoing);
    }

    void receiveMessage(const std::shared_ptr<serial_interfaces::srv::Receive::Response> response){
        if(!portIsOpen)
            xlr->begin();
        
        response->incoming = xlr->read();
        std::cout << response->incoming << std::endl;
    }

    void converseWithTerminal(const std::shared_ptr<serial_interfaces::srv::Converse::Request> request, std::shared_ptr<serial_interfaces::srv::Converse::Response> response){
        if(!portIsOpen)
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
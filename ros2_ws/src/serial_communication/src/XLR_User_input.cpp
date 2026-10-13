#include <cstdio>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "serial_interfaces/srv/converse.hpp"
#include "rclcpp/rclcpp.hpp"

using std::placeholders::_1;
using Converse = serial_interfaces::srv::Converse;

class XLR_UI : public rclcpp::Node {

    std::string message;

public: 

    XLR_UI() : Node("XLR_UI") {
        ConverseClient = this->create_client<Converse> ("Converse");

        while(true){
            std::cin >> message;
            converse(message);
        }
    }

private:

    void converse_response(const rclcpp::Client<Converse>::SharedFuture future){
        auto response = future.get();

        std::cout << response->incoming << '\n';
    }

    void converse(std::string message){
        auto request = std::make_shared<Converse::Request>();
        request->outgoing = message + '\n';

        ConverseClient->async_send_request(request, std::bind(&XLR_UI::converse_response, this, _1));
    }

    rclcpp::Client<Converse>::SharedPtr ConverseClient;
};




int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<XLR_UI>());
    rclcpp::shutdown();

    return 0;
}
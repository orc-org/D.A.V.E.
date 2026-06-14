#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"
#include <string>
#include <cmath>

class StepperDriverNode : public rclcpp::Node
{
public:
    StepperDriverNode() : Node("stepper_driver_node")
    {
        // parameter for max step frequency (Hz) at 100% velocity input
        this->declare_parameter<double>("max_step_frequency", 2000.0);
        max_step_frequency_ = this->get_parameter("max_step_frequency").as_double();

        // subscribe to the 4 independent wheel motor velocity command topics
        fl_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/front_left", 10,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                processMotorCommand("Front Left", msg->data, fl_freq_, fl_dir_);
            });

        fr_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/front_right", 10,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                processMotorCommand("Front Right", msg->data, fr_freq_, fr_dir_);
            });

        rl_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/rear_left", 10,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                processMotorCommand("Rear Left", msg->data, rl_freq_, rl_dir_);
            });

        rr_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/rear_right", 10,
            [this](const std_msgs::msg::Float32::SharedPtr msg) {
                processMotorCommand("Rear Right", msg->data, rr_freq_, rr_dir_);
            });

        RCLCPP_INFO(this->get_logger(), "Stepper Motor Driver Skeleton Node Initialized.");
        RCLCPP_INFO(this->get_logger(), "Max Step Frequency configured to: %.1f Hz", max_step_frequency_);
    }

private:
    void processMotorCommand(const std::string &wheel_name, float cmd, double &freq_out, std::string &dir_out)
    {
        // clamp command to safeguard against out of range values
        float velocity = std::max(-1.0f, std::min(1.0f, cmd));

        // determine step frequency (Hz)
        freq_out = std::abs(velocity) * max_step_frequency_;

        // determine direction pin logical state
        dir_out = (velocity >= 0.0f) ? "HIGH (FORWARD)" : "LOW (REVERSE)";

        // log the driver pulse conversions at a throttled rate
        RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
            "[%s Wheel] -> Converting logical command: %.2f | Stepper Signal: Freq = %.1f Hz, Dir Pin = %s", 
            wheel_name.c_str(), velocity, freq_out, dir_out.c_str());
    }

    double max_step_frequency_;

    // current states
    double fl_freq_ = 0.0, fr_freq_ = 0.0, rl_freq_ = 0.0, rr_freq_ = 0.0;
    std::string fl_dir_ = "HIGH", fr_dir_ = "HIGH", rl_dir_ = "HIGH", rr_dir_ = "HIGH";

    // subscriptions
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr fl_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr fr_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr rl_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr rr_sub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StepperDriverNode>());
    rclcpp::shutdown();
    return 0;
}

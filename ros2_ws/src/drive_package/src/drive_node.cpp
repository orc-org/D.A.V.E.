#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "std_msgs/msg/float32.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <algorithm>

class JoyDrive : public rclcpp::Node
{
public:
    JoyDrive() : Node("drive_node")
    {
        joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "/joy", 10,
            std::bind(&JoyDrive::joyCallback, this, std::placeholders::_1));

        cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel", 10,
            std::bind(&JoyDrive::cmdVelCallback, this, std::placeholders::_1));

        front_left_pub_  = this->create_publisher<std_msgs::msg::Float32>("/motor/front_left", 10); 
        front_right_pub_ = this->create_publisher<std_msgs::msg::Float32>("/motor/front_right", 10);
        rear_left_pub_   = this->create_publisher<std_msgs::msg::Float32>("/motor/rear_left", 10);
        rear_right_pub_  = this->create_publisher<std_msgs::msg::Float32>("/motor/rear_right", 10);

        RCLCPP_INFO(this->get_logger(), "Drive node running");
    }

private:
    void joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg)
    {
        // Grabbing Controller Inputs
        float steering = msg->axes[0];   // left stick (X axis only for left/right)
        float lt_raw = msg->axes[4];     // left trigger
        float rt_raw = msg->axes[5];     // right trigger

        //Variables to be defined later
        float left = 0.0f;
        float right = 0.0f;
        float spin = 0.0f;

        // Checking to see what is being picked up from controller (Not required, but made debugging much easier)
        RCLCPP_INFO(this->get_logger(), "\n\n\tController Inputs (For Debugging): \n\n Left Trigger: %.3f, Right Trigger: %.3f, LStick X Direction: %.3f\n", lt_raw, rt_raw, steering);

        // NORMALIZE trigger inputs (released=1, pressed=-1 → 0..1)
        float lt_norm = (1.0f - lt_raw) * 0.5f;   // backward  (0..1)
        float rt_norm = (1.0f - rt_raw) * 0.5f;   // forward   (0..1)
        
        // Combines it into Throttle to send to motor drivers (for variable speeds)
        float throttle = rt_norm - lt_norm;            // -1..1
        
        // Makes a deadzone to hopefully stop all the fluctuating values Im getting
        float deadzone = 0.05f;           // 5% deadzone around 0
        if (std::abs(throttle) < deadzone) {
            throttle = 0.0f;               
        }
        
        // If triggers are held

        if (throttle !=0.0f) {
        // --- Power Mixing ---
        left  = throttle * (1.0f - steering);
        right = throttle * (1.0f + steering);
        }

        // Fail-safe spin maneuver Will make only stick input spin rover.
        // (incase one side is not on flat terrain and trying to turn)
        else{

            spin = steering;
            //Adds deadzone that only affects the spinning (hopefully)
            if (std::abs(spin) < deadzone) {
                spin = 0.0f;               
            }

            left = -spin;
            right = spin;
        }

        // Clamp makes sure that our power outputs to motors never go above 100%
        left  = std::clamp(left,  -1.0f, 1.0f);
        right = std::clamp(right, -1.0f, 1.0f);
        // Could also use these to adjust sensitivity of movement instead of adaptive trigger resistance
        // (Would just cap acceleration to like 50%, make it so bumpers inc/dec the clamp values)

        // Send stuff to the topics
        std_msgs::msg::Float32 flmsg, frmsg, rlmsg, rrmsg;
        flmsg.data = left;
        rlmsg.data = left;
        frmsg.data = right;
        rrmsg.data = right;

        front_left_pub_->publish(flmsg);
        front_right_pub_->publish(frmsg);
        rear_left_pub_->publish(rlmsg);
        rear_right_pub_->publish(rrmsg);
        
        // Tells user the power % to each motor
        RCLCPP_INFO(this->get_logger(), "\n\n\tMotor Power Levels: \n\nFront Left: %d%% | Front Right: %d%%\nRear Left:  %d%% | Rear Right:  %d%%\n", 
                    static_cast<int>(left*100.0), static_cast<int>(right*100.0),
                    static_cast<int>(left*100.0), static_cast<int>(right*100.0));

    }

    void cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        float throttle = msg->linear.x;
        float steering = msg->angular.z;

        // Simple skid-steer mixing
        float left = throttle - steering;
        float right = throttle + steering;

        left  = std::clamp(left,  -1.0f, 1.0f);
        right = std::clamp(right, -1.0f, 1.0f);

        std_msgs::msg::Float32 flmsg, frmsg, rlmsg, rrmsg;
        flmsg.data = left;
        rlmsg.data = left;
        frmsg.data = right;
        rrmsg.data = right;

        front_left_pub_->publish(flmsg);
        front_right_pub_->publish(frmsg);
        rear_left_pub_->publish(rlmsg);
        rear_right_pub_->publish(rrmsg);

        RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
            "cmd_vel -> Left Wheels: %.2f | Right Wheels: %.2f", left, right);
    }

    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr front_left_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr front_right_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr rear_left_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr rear_right_pub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JoyDrive>());
    rclcpp::shutdown();
    return 0;
}

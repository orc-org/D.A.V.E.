import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer
from sensor_msgs.msg import JointState
from std_msgs.msg import Float32
from arm_interfaces.action import GripperCommand
import time

class ArmStepperDriverNode(Node):
    def __init__(self):
        super().__init__('arm_stepper_driver')

        # stepper configuration parameters, I have NO idea if any of these are right but we can change them later
        self.declare_parameter('arm_gear_ratio', 50.0)      # 50:1 gearbox on arm joint
        self.declare_parameter('arm_motor_steps', 200)      # 200 steps per rev (1.8 deg)
        self.declare_parameter('gripper_lead_pitch', 0.008)  # 8mm travel per rev (lead screw)
        self.declare_parameter('gripper_motor_steps', 200)  # 200 steps per rev (1.8 deg)

        self.gear_ratio = self.get_parameter('arm_gear_ratio').value
        self.arm_motor_steps = self.get_parameter('arm_motor_steps').value
        self.gripper_pitch = self.get_parameter('gripper_lead_pitch').value
        self.gripper_motor_steps = self.get_parameter('gripper_motor_steps').value

        # arm steps per radian = (gear_ratio * motor_steps) / 2pi
        self.arm_steps_per_rad = (self.gear_ratio * self.arm_motor_steps) / (2.0 * 3.14159265359)
        # gripper steps per meter = motor_steps / pitch
        self.gripper_steps_per_m = self.gripper_motor_steps / self.gripper_pitch

        # subscribe to joint states to monitor arm angle
        self.joint_sub = self.create_subscription(
            JointState,
            '/joint_states',
            self.joint_state_callback,
            10
        )

        # publisher for gripper state (for simulation visualizer)
        self.gripper_state_pub = self.create_publisher(Float32, '/gripper_state', 10)

        # subscribe to gripper target topic
        self.gripper_target_sub = self.create_subscription(
            Float32,
            '/gripper_target',
            self.gripper_target_callback,
            10
        )

        # action Server for GripperCommand
        self.gripper_action_server = ActionServer(
            self,
            GripperCommand,
            'gripper_command',
            self.execute_gripper_goal
        )

        # track internal state
        self.current_arm_angle = 0.0
        self.target_gripper_pos = 0.0   # 0.0 = fully open, 1.0 = fully closed
        self.current_gripper_pos = 0.0  # 0.0 = fully open (spacing 0.2m), 1.0 = fully closed (spacing 0.0m)
        self.last_log_time = 0.0

        # 10Hz timer loop to simulate physical stepper motor rotation and travel over time
        self.gripper_timer = self.create_timer(0.1, self.update_gripper_loop)

        self.get_logger().info('Arm & Gripper Stepper Driver Node Running.')
        self.get_logger().info(f'Arm conversion: {self.arm_steps_per_rad:.2f} steps/rad')
        self.get_logger().info(f'Gripper conversion: {self.gripper_steps_per_m:.2f} steps/meter')

        # publish initial state
        self.publish_gripper_state()

    def publish_gripper_state(self):
        state_msg = Float32()
        state_msg.data = self.current_gripper_pos
        self.gripper_state_pub.publish(state_msg)

    def joint_state_callback(self, msg):
        # find arm_joint index
        if 'arm_joint' in msg.name:
            idx = msg.name.index('arm_joint')
            self.current_arm_angle = msg.position[idx]
            
            # convert to steps
            arm_steps = int(self.current_arm_angle * self.arm_steps_per_rad)
            
            # throttle log output to once per second
            now = self.get_clock().now().nanoseconds / 1e9
            if now - self.last_log_time >= 1.0:
                self.last_log_time = now
                self.get_logger().info(
                    f'Arm angle: {self.current_arm_angle:.3f} rad | Converted Stepper: {arm_steps} steps'
                )

    def gripper_target_callback(self, msg):
        # update target gripper position
        self.target_gripper_pos = max(0.0, min(1.0, msg.data))

    def update_gripper_loop(self):
        # step the gripper position towards the target at 1.0 units per second (0.1 per tick)
        step_delta = 0.1
        if abs(self.current_gripper_pos - self.target_gripper_pos) > 0.01:
            if self.current_gripper_pos < self.target_gripper_pos:
                self.current_gripper_pos = min(self.target_gripper_pos, self.current_gripper_pos + step_delta)
            else:
                self.current_gripper_pos = max(self.target_gripper_pos, self.current_gripper_pos - step_delta)
            
            self.publish_gripper_state()
            
            # calculate physical linear distance (finger opening spacing)
            # 0.0 open = 0.2m linear offset, 1.0 closed = 0.0m linear offset
            finger_linear_pos = (1.0 - self.current_gripper_pos) * 0.2
            gripper_steps = int(finger_linear_pos * self.gripper_steps_per_m)
            
            self.get_logger().info(
                f'Gripper position: {self.current_gripper_pos:.2f} | Linear spacing: {finger_linear_pos:.3f}m | Stepper: {gripper_steps} steps'
            )

    def execute_gripper_goal(self, goal_handle):
        target_pos = goal_handle.request.position
        target_pos = max(0.0, min(1.0, target_pos))
        self.target_gripper_pos = target_pos
        
        self.get_logger().info(f'Action Gripper Command received: moving to {target_pos:.2f}')
        
        feedback_msg = GripperCommand.Feedback()
        rate = self.create_rate(10)  # 10Hz
        
        # wait for the timer loop to complete the movement
        while abs(self.current_gripper_pos - target_pos) > 0.01:
            feedback_msg.progress = self.current_gripper_pos
            goal_handle.publish_feedback(feedback_msg)
            rate.sleep()
            
        goal_handle.succeed()
        
        result = GripperCommand.Result()
        result.success = True
        return result

def main(args=None):
    rclpy.init(args=args)
    node = ArmStepperDriverNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()

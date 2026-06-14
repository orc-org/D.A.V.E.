import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer
from geometry_msgs.msg import Point, Twist, TransformStamped
from sensor_msgs.msg import JointState
from std_msgs.msg import Float32, Bool
from tf2_ros import TransformBroadcaster
from ament_index_python.packages import get_package_share_directory

from arm_interfaces.msg import ArmStatus
from arm_interfaces.srv import CalibrateArm
from arm_interfaces.action import MoveToPoint

import os
import yaml
import math
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Slider
from arm_controller.kinematics import Component3D
import threading
import time

class MotionCoordinatorNode(Node):
    def __init__(self):
        super().__init__('motion_coordinator')
        
        # load parameters from YAML
        config_path = os.path.join(
            get_package_share_directory('rover_description'),
            'config', 'physical_params.yaml')
        
        with open(config_path, 'r') as f:
            params = yaml.safe_load(f)['robot_dimensions']
        
        # initialize kinematic chain using loaded params
        self.base = Component3D("Rover Base", limit_x=(0,0), limit_y=(0,0))
        self.column = Component3D("Column", parent=self.base, offset_z=params['column_base_offset'], fixed=True)
        self.mount = Component3D("Mount", parent=self.column, offset_z=params['column_height'], fixed=True)
        self.wrist = Component3D("Wrist", parent=self.mount, offset_x=params['mount_length'], 
                                 limit_x=(0,0), limit_y=(-90, 90), limit_z=(0,0))
        self.tool = Component3D("Tool Tip", parent=self.wrist, offset_x=params['arm_length'], fixed=True)
        
        self.declare_parameter('show_plot', False)
        self.show_plot = self.get_parameter('show_plot').value

        self.target_x, self.target_y, self.target_z = 25.0, 0.0, 2.0
        
        # simulation joint tracking
        self.fl_wheel_angle = 0.0
        self.fr_wheel_angle = 0.0
        self.rl_wheel_angle = 0.0
        self.rr_wheel_angle = 0.0
        self.gripper_pos = 0.0

        self.fl_vel = 0.0
        self.fr_vel = 0.0
        self.rl_vel = 0.0
        self.rr_vel = 0.0
        
        # smoothed velocity commands to prevent motor power jittering
        self.cmd_linear_x = 0.0
        self.cmd_angular_z = 0.0
        
        # topics
        self.subscription = self.create_subscription(Point, 'arm_target', self.target_callback, 10)
        self.status_pub = self.create_publisher(ArmStatus, 'arm_status', 10)
        self.global_pos_pub = self.create_publisher(Point, 'arm_global_pos', 10)
        self.cmd_vel_pub = self.create_publisher(Twist, 'cmd_vel', 10)
        self.joint_state_pub = self.create_publisher(JointState, 'joint_states', 10)
        self.tf_broadcaster = TransformBroadcaster(self)

        # motor commands & gripper feedback subscribers for wheel/finger visualization
        self.fl_motor_sub = self.create_subscription(Float32, '/motor/front_left', self.fl_motor_cb, 10)
        self.fr_motor_sub = self.create_subscription(Float32, '/motor/front_right', self.fr_motor_cb, 10)
        self.rl_motor_sub = self.create_subscription(Float32, '/motor/rear_left', self.rl_motor_cb, 10)
        self.rr_motor_sub = self.create_subscription(Float32, '/motor/rear_right', self.rr_motor_cb, 10)
        self.gripper_state_sub = self.create_subscription(Float32, '/gripper_state', self.gripper_state_cb, 10)

        self.auto_mode = True
        self.auto_mode_sub = self.create_subscription(Bool, '/auto_mode', self.auto_mode_cb, 10)

        self.arm_manual_vel = 0.0
        self.arm_manual_vel_sub = self.create_subscription(Float32, '/arm_manual_vel', self.arm_manual_vel_cb, 10)
        
        # services
        self.calibrate_srv = self.create_service(CalibrateArm, 'calibrate_arm', self.handle_calibrate)
        
        # actions
        self._action_server = ActionServer(self, MoveToPoint, 'move_to_point', execute_callback=self.execute_move_goal)

        self.timer_period = 0.05
        self.timer = self.create_timer(self.timer_period, self.update_loop)
        self.get_logger().info('Arm Controller: All Interfaces (Topic/Srv/Action) Loaded')

    def target_callback(self, msg):
        self.target_x, self.target_y, self.target_z = msg.x, msg.y, msg.z

    def fl_motor_cb(self, msg): self.fl_vel = msg.data
    def fr_motor_cb(self, msg): self.fr_vel = msg.data
    def rl_motor_cb(self, msg): self.rl_vel = msg.data
    def rr_motor_cb(self, msg): self.rr_vel = msg.data
    def gripper_state_cb(self, msg): self.gripper_pos = msg.data

    def arm_manual_vel_cb(self, msg):
        self.arm_manual_vel = msg.data

    def auto_mode_cb(self, msg):
        self.auto_mode = msg.data
        self.get_logger().info(f"Control Mode changed: {'AUTO (IK)' if self.auto_mode else 'MANUAL'}")

    def update_loop(self):
        # calculate manual drive speeds from subscriptions
        left_speed = (self.fl_vel + self.rl_vel) * 0.5
        right_speed = (self.fr_vel + self.rr_vel) * 0.5

        # scale speed for visual simulation
        scale = 3.0 
        linear_vel = (left_speed + right_speed) * 0.5 * scale
        angular_vel = (right_speed - left_speed) * 0.5 * scale

        # update base position from wheel telemetry integration
        self.base.angle_z += angular_vel * self.timer_period
        self.base.offset_x += linear_vel * math.cos(self.base.angle_z) * self.timer_period
        self.base.offset_y += linear_vel * math.sin(self.base.angle_z) * self.timer_period

        if self.auto_mode:
            # save original base pose before running IK
            orig_x = self.base.offset_x
            orig_y = self.base.offset_y
            orig_yaw = self.base.angle_z
            
            # run IK to find desired deltas
            linear_delta, angular_delta = self.base.inverse_kinematics(
                self.tool, self.target_x, self.target_y, self.target_z, 
                learning_rate=2.0, max_iterations=20, move_base=True)
            
            # restore base pose in simulation so it is only updated by wheel integration
            self.base.offset_x = orig_x
            self.base.offset_y = orig_y
            self.base.angle_z = orig_yaw
            
            # calculate target velocities
            target_linear = linear_delta / self.timer_period
            target_angular = angular_delta / self.timer_period
            
            # clamp to safe physical limits (m/s and rad/s)
            max_linear = 0.8
            max_angular = 1.0
            target_linear = max(-max_linear, min(max_linear, target_linear))
            target_angular = max(-max_angular, min(max_angular, target_angular))
            
            # low-pass filter (exponential smoothing) to prevent jitter
            alpha = 0.15 # smoothing factor (lower = smoother, higher = faster response)
            self.cmd_linear_x = alpha * target_linear + (1 - alpha) * self.cmd_linear_x
            self.cmd_angular_z = alpha * target_angular + (1 - alpha) * self.cmd_angular_z
            
            # publish Twist
            twist = Twist()
            twist.linear.x = self.cmd_linear_x
            twist.angular.z = self.cmd_angular_z
            self.cmd_vel_pub.publish(twist)
        else:
            # manual mode: update arm joint angle from manual velocity command
            if abs(self.arm_manual_vel) > 0.01:
                current_deg = math.degrees(self.wrist.angle_y)
                new_deg = current_deg + math.degrees(self.arm_manual_vel) * self.timer_period
                self.wrist.set_angles(ay=new_deg)
            
            # reset filtered commands in manual mode
            self.cmd_linear_x = 0.0
            self.cmd_angular_z = 0.0
        # update 3D model joint (and simulate wheel spin + gripper move)
        self.fl_wheel_angle = (self.fl_wheel_angle + self.fl_vel * 5.0 * self.timer_period) % (2.0 * math.pi)
        self.fr_wheel_angle = (self.fr_wheel_angle + self.fr_vel * 5.0 * self.timer_period) % (2.0 * math.pi)
        self.rl_wheel_angle = (self.rl_wheel_angle + self.rl_vel * 5.0 * self.timer_period) % (2.0 * math.pi)
        self.rr_wheel_angle = (self.rr_wheel_angle + self.rr_vel * 5.0 * self.timer_period) % (2.0 * math.pi)

        # calculate stepper motor shaft angle based on gripper travel spacing
        # pitch of 8mm travel per rev gives 25 revs total for the 0.2m linear range
        shaft_angle = (self.gripper_pos * 25.0 * 2.0 * math.pi) % (2.0 * math.pi)

        js = JointState()
        js.header.stamp = self.get_clock().now().to_msg()
        js.name = [
            'arm_joint',
            'front_left_wheel_joint',
            'front_right_wheel_joint',
            'rear_left_wheel_joint',
            'rear_right_wheel_joint',
            'left_finger_joint',
            'right_finger_joint',
            'gripper_motor_shaft_joint'
        ]
        # prismatic finger limit mapping: moves inward (negative direction)
        finger_val = -self.gripper_pos * 0.2
        js.position = [
            float(self.wrist.angle_y),
            float(self.fl_wheel_angle),
            float(self.fr_wheel_angle),
            float(self.rl_wheel_angle),
            float(self.rr_wheel_angle),
            float(finger_val),
            float(finger_val),
            float(shaft_angle)
        ]
        self.joint_state_pub.publish(js)

        # update 3D model base position (TF)
        t = TransformStamped()
        t.header.stamp = js.header.stamp
        t.header.frame_id = 'world'
        t.child_frame_id = 'base_link'
        t.transform.translation.x = float(self.base.offset_x)
        t.transform.translation.y = float(self.base.offset_y)
        t.transform.translation.z = 0.0
        t.transform.rotation.z = math.sin(self.base.angle_z / 2.0)
        t.transform.rotation.w = math.cos(self.base.angle_z / 2.0)
        self.tf_broadcaster.sendTransform(t)

        # publish status and global position
        gx, gy, gz = self.tool.get_global_position()
        self.global_pos_pub.publish(Point(x=float(gx), y=float(gy), z=float(gz)))
        
        dist = math.sqrt((self.target_x - gx)**2 + (self.target_y - gy)**2 + (self.target_z - gz)**2)
        status_msg = ArmStatus()
        status_msg.current_dist = dist
        self.status_pub.publish(status_msg)

    def handle_calibrate(self, request, response):
        self.get_logger().info('Calibrating...')
        self.base.set_position(0.0, 0.0, 0.0)
        self.base.angle_z = 0.0
        self.wrist.angle_y = 0.0
        response.success = True
        return response

    async def execute_move_goal(self, goal_handle):
        req = goal_handle.request.target
        self.target_x, self.target_y, self.target_z = req.x, req.y, req.z
        self.get_logger().info(f'Action: Moving to {self.target_x}, {self.target_y}')
        goal_handle.succeed()
        return MoveToPoint.Result(success=True)

    def draw_state(self, ax):
        ax.cla()
        self.base.draw(ax)
        ax.plot([self.target_x], [self.target_y], [self.target_z], 'g*', markersize=15)
        r = max(abs(self.target_x), abs(self.target_y), abs(self.target_z), 50.0) + 10.0
        ax.set_xlim(-r, r); ax.set_ylim(-r, r); ax.set_zlim(-r, r)

def main(args=None):
    rclpy.init(args=args)
    node = MotionCoordinatorNode()
    
    if node.show_plot:
        spin_thread = threading.Thread(target=rclpy.spin, args=(node,), daemon=True)
        spin_thread.start()
        fig = plt.figure(figsize=(10, 8))
        ax = fig.add_subplot(111, projection='3d')
        ani = FuncAnimation(fig, lambda i: node.draw_state(ax), interval=50)
        plt.show()
    else:
        rclpy.spin(node)
        
    rclpy.shutdown()

if __name__ == '__main__':
    main()

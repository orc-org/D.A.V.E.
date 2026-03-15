import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer
from geometry_msgs.msg import Point, Twist, TransformStamped
from sensor_msgs.msg import JointState
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
        
        # 1. Load Parameters from YAML (SINGLE SOURCE OF TRUTH)
        config_path = os.path.join(
            get_package_share_directory('rover_description'),
            'config', 'physical_params.yaml')
        
        with open(config_path, 'r') as f:
            params = yaml.safe_load(f)['robot_dimensions']
        
        # 2. Initialize Kinematic Chain using loaded params
        self.base = Component3D("Rover Base", limit_x=(0,0), limit_y=(0,0))
        self.column = Component3D("Column", parent=self.base, offset_z=params['column_base_offset'], fixed=True)
        self.mount = Component3D("Mount", parent=self.column, offset_z=params['column_height'], fixed=True)
        self.wrist = Component3D("Wrist", parent=self.mount, offset_x=params['mount_length'], 
                                 limit_x=(0,0), limit_y=(-90, 90), limit_z=(0,0))
        self.tool = Component3D("Tool Tip", parent=self.wrist, offset_x=params['arm_length'], fixed=True)
        
        self.target_x, self.target_y, self.target_z = 25.0, 0.0, 2.0
        
        # --- TOPIC SECTION ---
        self.subscription = self.create_subscription(Point, 'arm_target', self.target_callback, 10)
        self.status_pub = self.create_publisher(ArmStatus, 'arm_status', 10)
        self.global_pos_pub = self.create_publisher(Point, 'arm_global_pos', 10)
        self.cmd_vel_pub = self.create_publisher(Twist, 'cmd_vel', 10)
        self.joint_state_pub = self.create_publisher(JointState, 'joint_states', 10)
        self.tf_broadcaster = TransformBroadcaster(self)
        
        # --- SERVICE SECTION ---
        self.calibrate_srv = self.create_service(CalibrateArm, 'calibrate_arm', self.handle_calibrate)
        
        # --- ACTION SECTION ---
        self._action_server = ActionServer(self, MoveToPoint, 'move_to_point', execute_callback=self.execute_move_goal)

        self.timer_period = 0.05
        self.timer = self.create_timer(self.timer_period, self.update_loop)
        self.get_logger().info('Arm Controller: All Interfaces (Topic/Srv/Action) Loaded')

    def target_callback(self, msg):
        self.target_x, self.target_y, self.target_z = msg.x, msg.y, msg.z

    def update_loop(self):
        linear_delta, angular_delta = self.base.inverse_kinematics(
            self.tool, self.target_x, self.target_y, self.target_z, 
            learning_rate=2.0, max_iterations=20, move_base=True)
        
        # 1. Update 3D Model Joint
        js = JointState()
        js.header.stamp = self.get_clock().now().to_msg()
        js.name = ['arm_joint']
        js.position = [float(self.wrist.angle_y)]
        self.joint_state_pub.publish(js)

        # 2. Update 3D Model Base Position (TF)
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

        # 3. Publish Velocity
        twist = Twist()
        twist.linear.x = linear_delta / self.timer_period
        twist.angular.z = angular_delta / self.timer_period
        self.cmd_vel_pub.publish(twist)
        
        # 4. Publish Status and Global Pos
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
        ax.set_xlim(-30, 30); ax.set_ylim(-30, 30); ax.set_zlim(0, 20)

def main(args=None):
    rclpy.init(args=args)
    node = MotionCoordinatorNode()
    spin_thread = threading.Thread(target=rclpy.spin, args=(node,), daemon=True)
    spin_thread.start()
    fig = plt.figure(figsize=(10, 8)); plt.subplots_adjust(bottom=0.25)
    ax = fig.add_subplot(111, projection='3d')
    ax_x = plt.axes([0.2, 0.15, 0.65, 0.03]); s_x = Slider(ax_x, 'X', -30, 30, valinit=node.target_x)
    ax_y = plt.axes([0.2, 0.10, 0.65, 0.03]); s_y = Slider(ax_y, 'Y', -30, 30, valinit=node.target_y)
    ax_z = plt.axes([0.2, 0.05, 0.65, 0.03]); s_z = Slider(ax_z, 'Z', 0, 20, valinit=node.target_z)
    def update_sliders(val): node.target_x, node.target_y, node.target_z = s_x.val, s_y.val, s_z.val
    s_x.on_changed(update_sliders); s_y.on_changed(update_sliders); s_z.on_changed(update_sliders)
    ani = FuncAnimation(fig, lambda i: node.draw_state(ax), interval=50)
    plt.show()
    rclpy.shutdown()

if __name__ == '__main__':
    main()

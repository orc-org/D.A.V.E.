from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='rover_ekf',
            executable='ekf_node',
            name='rover_ekf_node',
            output='screen',
            parameters=[{
                'frequency': 20.0,
                'topic_imu_euler': '/imu/euler',
                'topic_imu_raw': '/imu/data_raw',
                'topic_gps_fix': '/gps/fix',
                'topic_cmd_vel': '/cmd_vel',
                'topic_ekf_odom': '/ekf/odometry',
                'topic_ekf_euler': '/ekf/filtered_euler',
                'frame_id': 'odom',
                'child_frame_id': 'base_link',
            }]
        )
    ])

import os
import xacro
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    package_name = 'rover_description'
    
    # path to Xacro file
    xacro_path = os.path.join(
        get_package_share_directory(package_name),
        'urdf',
        'rover.urdf.xacro')

    # process Xacro to generate URDF
    robot_description_raw = xacro.process_file(xacro_path).toxml()

    # path to RViz Config
    rviz_config_path = os.path.join(
        get_package_share_directory(package_name),
        'rviz',
        'default.rviz')

    return LaunchDescription([
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            parameters=[{'robot_description': robot_description_raw}]
        ),
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_path]
        )
    ])

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # declare arguments
    sim_mode_arg = DeclareLaunchArgument(
        'sim_mode',
        default_value='false',
        description='run imu telemetry in simulation mode if true'
    )
    i2c_bus_arg = DeclareLaunchArgument(
        'i2c_bus',
        default_value='7',
        description='i2c bus number for bno085'
    )
    publish_rate_arg = DeclareLaunchArgument(
        'publish_rate',
        default_value='20.0',
        description='telemetry publish rate in hz'
    )

    # telemetry node
    telemetry_node = Node(
        package='imu',
        executable='telemetry',
        name='imu_telemetry',
        output='screen',
        parameters=[{
            'sim_mode': LaunchConfiguration('sim_mode'),
            'i2c_bus': LaunchConfiguration('i2c_bus'),
            'publish_rate': LaunchConfiguration('publish_rate'),
        }]
    )

    # kalman filter node
    kalman_filter_node = Node(
        package='imu',
        executable='kalman_filter',
        name='imu_kalman_filter',
        output='screen',
        parameters=[{
            'topic_raw_imu': '/imu/data_raw',
            'topic_filtered_imu': '/imu/data_filtered',
            'topic_odom': '/imu/odometry',
        }]
    )

    return LaunchDescription([
        sim_mode_arg,
        i2c_bus_arg,
        publish_rate_arg,
        telemetry_node,
        kalman_filter_node,
    ])

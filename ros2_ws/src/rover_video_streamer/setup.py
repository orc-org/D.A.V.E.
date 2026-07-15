from setuptools import find_packages, setup

package_name = 'rover_video_streamer'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='orc',
    maintainer_email='orc@example.com',
    description='Low-latency GStreamer-based video streaming package for ROS 2 and Jetson Orin Nano',
    license='Apache-2.0',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'video_streamer = rover_video_streamer.video_streamer:main'
        ],
    },
)

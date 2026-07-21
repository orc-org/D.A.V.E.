from setuptools import setup, find_packages
import os
from glob import glob

package_name = 'imu'

setup(
    name=package_name,
    version='0.1.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob('launch/*.launch.py')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='orc',
    maintainer_email='orc@example.com',
    description='ROS 2 package for BNO085 IMU telemetry and Unscented Kalman Filtering',
    license='Apache-2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'telemetry = imu.telemetry:main',
            'kalman_filter = imu.kalman_filter:main',
        ],
    },
)

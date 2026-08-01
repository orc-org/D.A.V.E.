import os
from glob import glob
from setuptools import find_packages, setup

package_name = 'rover_ekf'

setup(
    name=package_name,
    version='1.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob('launch/*.launch.py')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='orc',
    maintainer_email='orc@todo.todo',
    description='Extended Kalman Filter (EKF) Sensor Fusion node for Rover GPS, IMU, and Wheel Odometry.',
    license='MIT',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'ekf_node = rover_ekf.ekf_node:main',
        ],
    },
)

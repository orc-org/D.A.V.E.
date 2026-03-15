# arm_controller

This package implements the Inverse Kinematics (IK) math and the ROS 2 coordination logic.

## Features
*   **1-DOF Driving Mode**: The rover base drives around to position itself because the arm only has one vertical degree of freedom.
*   **Hybrid IK**: Combines gradient descent for the arm joint with direct geometry for non-[holonomic](https://en.wikipedia.org/wiki/Holonomic_constraints) base navigation.
*   **Dynamic Configuration**: Automatically loads physical dimensions from the `rover_description` package.

## Core Files
*   **`kinematics.py`**: Implementation of homogeneous transformations and gradient-descent IK.
*   **`motion_coordinator_node.py`**: ROS2 node that bridges RViz and external topics.

## ROS 2 Interface
*   **Subscribers**: `/arm_target` (`geometry_msgs/Point`, Where the arm needs to go)
*   **Publishers**: `/cmd_vel` (Driving commands), `/joint_states` (Arm visual/Current angle), `/arm_global_pos` (Tooltip location).
*   **Services**: `/calibrate_arm` (Reset logic).

## Running
```bash
ros2 run arm_controller motion_coordinator
```
*(Ensure `rover_description` is built first so config can be loaded)*

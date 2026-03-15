# rover_description

This package defines the physical identity of the rover. It contains the 3D models (URDF), configuration files, and visualization settings.

## Single Source of Truth
Physical dimensions (column height, arm length, etc.) are stored in:
`config/physical_params.yaml`

**(So far) Both the 3D Model and the Python IK math read from this file.** Change it here to update the entire rover.

## Contents
*   **`urdf/rover.urdf.xacro`**: The 3D blueprint. It processes the YAML config to generate the final model.
*   **`launch/view_robot.launch.py`**: Starts the robot state publisher and RViz2.
*   **`rviz/default.rviz`**: Saved camera and display settings for a "one-click" setup.

## Visualization
To see the robot in 3D (rviz):
```bash
ros2 launch rover_description view_robot.launch.py
```
*(Remember to set Fixed Frame to `world` if running with the arm_controller)*

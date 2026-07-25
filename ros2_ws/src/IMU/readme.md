# IMU ROS 2 Package (`imu`)

ROS 2 package for reading telemetry from the BNO085 IMU over I2C and estimating state using a 15-dimensional Unscented Kalman Filter (`pykalman`).

---

## 1. System Setup & Dependencies

```bash
sudo apt-get update
sudo apt-get install -y i2c-tools python3-pip
sudo usermod -aG i2c $USER

pip3 install adafruit-circuitpython-bno08x pykalman numpy matplotlib adafruit-extended-bus
```

---

## 2. Build & Launch

```bash
cd ~/D.A.V.E./ros2_ws
source /opt/ros/humble/setup.bash
colcon build --packages-select imu
source install/setup.bash

# Launch both nodes (hardware mode)
ros2 launch imu imu.launch.py

# Launch both nodes (simulation mode)
ros2 launch imu imu.launch.py sim_mode:=true
```

---

## 3. Extractable Topics & Subfield Reference

Below is the complete list of all topics published by the package, their message types, and the subfields you can extract using `ros2 topic echo <topic> --field <subfield>`.

### A. Raw Telemetry Node (`imu_telemetry`)

#### 1. `/imu/data_raw` (`sensor_msgs/msg/Imu`)
- `header`
  - `header.stamp.sec` (timestamp seconds)
  - `header.stamp.nanosec` (timestamp nanoseconds)
  - `header.frame_id` (frame identifier, e.g. `imu_link`)
- `linear_acceleration` (Linear acceleration in m/s²)
  - `linear_acceleration.x` $\rightarrow$ `ros2 topic echo /imu/data_raw --field linear_acceleration.x`
  - `linear_acceleration.y` $\rightarrow$ `ros2 topic echo /imu/data_raw --field linear_acceleration.y`
  - `linear_acceleration.z` $\rightarrow$ `ros2 topic echo /imu/data_raw --field linear_acceleration.z`
- `angular_velocity` (Angular velocity in rad/s)
  - `angular_velocity.x` $\rightarrow$ `ros2 topic echo /imu/data_raw --field angular_velocity.x`
  - `angular_velocity.y` $\rightarrow$ `ros2 topic echo /imu/data_raw --field angular_velocity.y`
  - `angular_velocity.z` $\rightarrow$ `ros2 topic echo /imu/data_raw --field angular_velocity.z`
- `orientation` (Quaternion orientation x, y, z, w)
  - `orientation.x` $\rightarrow$ `ros2 topic echo /imu/data_raw --field orientation.x`
  - `orientation.y` $\rightarrow$ `ros2 topic echo /imu/data_raw --field orientation.y`
  - `orientation.z` $\rightarrow$ `ros2 topic echo /imu/data_raw --field orientation.z`
  - `orientation.w` $\rightarrow$ `ros2 topic echo /imu/data_raw --field orientation.w`
- `linear_acceleration_covariance` ($3 \times 3$ row-major 9-element array)
- `angular_velocity_covariance` ($3 \times 3$ row-major 9-element array)
- `orientation_covariance` ($3 \times 3$ row-major 9-element array)

#### 2. `/imu/euler` (`geometry_msgs/msg/Vector3`)
- `x` (Roll angle in degrees) $\rightarrow$ `ros2 topic echo /imu/euler --field x`
- `y` (Pitch angle in degrees) $\rightarrow$ `ros2 topic echo /imu/euler --field y`
- `z` (Yaw angle in degrees) $\rightarrow$ `ros2 topic echo /imu/euler --field z`

#### 3. `/imu/telemetry_str` (`std_msgs/msg/String`)
- `data` (Formatted multi-line text string of telemetry) $\rightarrow$ `ros2 topic echo /imu/telemetry_str --field data`

---

### B. Kalman Filter Node (`imu_kalman_filter`)

#### 1. `/imu/data_filtered` (`sensor_msgs/msg/Imu`)
- `linear_acceleration` (Filtered linear acceleration in m/s²)
  - `linear_acceleration.x` $\rightarrow$ `ros2 topic echo /imu/data_filtered --field linear_acceleration.x`
  - `linear_acceleration.y` $\rightarrow$ `ros2 topic echo /imu/data_filtered --field linear_acceleration.y`
  - `linear_acceleration.z` $\rightarrow$ `ros2 topic echo /imu/data_filtered --field linear_acceleration.z`
- `angular_velocity` (Filtered angular velocity in rad/s)
  - `angular_velocity.x` $\rightarrow$ `ros2 topic echo /imu/data_filtered --field angular_velocity.x`
  - `angular_velocity.y` $\rightarrow$ `ros2 topic echo /imu/data_filtered --field angular_velocity.y`
  - `angular_velocity.z` $\rightarrow$ `ros2 topic echo /imu/data_filtered --field angular_velocity.z`
- `orientation` (Filtered quaternion orientation)
  - `orientation.x`, `orientation.y`, `orientation.z`, `orientation.w`
- `linear_acceleration_covariance` ($3 \times 3$ covariance matrix from UKF)
- `angular_velocity_covariance` ($3 \times 3$ covariance matrix from UKF)
- `orientation_covariance` ($3 \times 3$ covariance matrix from UKF)

#### 2. `/imu/odometry` (`nav_msgs/msg/Odometry`)
- `pose.pose.position` (Estimated position in meters)
  - `pose.pose.position.x` $\rightarrow$ `ros2 topic echo /imu/odometry --field pose.pose.position.x`
  - `pose.pose.position.y` $\rightarrow$ `ros2 topic echo /imu/odometry --field pose.pose.position.y`
  - `pose.pose.position.z` $\rightarrow$ `ros2 topic echo /imu/odometry --field pose.pose.position.z`
- `pose.pose.orientation` (Filtered orientation quaternion)
  - `pose.pose.orientation.x`, `pose.pose.orientation.y`, `pose.pose.orientation.z`, `pose.pose.orientation.w`
- `twist.twist.linear` (Estimated linear velocity in m/s)
  - `twist.twist.linear.x` $\rightarrow$ `ros2 topic echo /imu/odometry --field twist.twist.linear.x`
  - `twist.twist.linear.y` $\rightarrow$ `ros2 topic echo /imu/odometry --field twist.twist.linear.y`
  - `twist.twist.linear.z` $\rightarrow$ `ros2 topic echo /imu/odometry --field twist.twist.linear.z`
- `twist.twist.angular` (Estimated angular velocity in rad/s)
  - `twist.twist.angular.x`, `twist.twist.angular.y`, `twist.twist.angular.z`
- `pose.covariance` ($6 \times 6$ 36-element pose covariance array)
- `twist.covariance` ($6 \times 6$ 36-element twist covariance array)

#### 3. `/imu/filtered_euler` (`geometry_msgs/msg/Vector3`)
- `x` (Filtered Roll angle in degrees) $\rightarrow$ `ros2 topic echo /imu/filtered_euler --field x`
- `y` (Filtered Pitch angle in degrees) $\rightarrow$ `ros2 topic echo /imu/filtered_euler --field y`
- `z` (Filtered Yaw angle in degrees) $\rightarrow$ `ros2 topic echo /imu/filtered_euler --field z`

---

## 4. Services & Parameters

### Services
```bash
# Re-initialize BNO085 hardware sensor
ros2 service call /imu_telemetry/calibrate std_srvs/srv/Trigger

# Pause/resume telemetry stream
ros2 service call /imu_telemetry/enable_publishing std_srvs/srv/SetBool "{data: true}"

# Query telemetry status
ros2 service call /imu_telemetry/get_status std_srvs/srv/Trigger

# Reset Kalman filter state mean and covariance
ros2 service call /imu_kalman_filter/reset_filter std_srvs/srv/Trigger

# Query 15D Kalman filter state vector summary
ros2 service call /imu_kalman_filter/get_state std_srvs/srv/Trigger
```

### Configurable ROS Parameters
- `i2c_bus` (int, default: `7`)
- `i2c_address` (int, default: `0x4A`)
- `publish_rate` (float, default: `20.0` Hz)
- `sim_mode` (bool, default: `false`)
- `dt` (float, default: `0.1` s)
- `process_noise` (float, default: `0.01`)
- `frame_id` (string, default: `'imu_link'`)
- `odom_frame_id` (string, default: `'odom'`)
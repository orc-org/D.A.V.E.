import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer
from sensor_msgs.msg import JointState
from std_msgs.msg import Float32
from arm_interfaces.action import GripperCommand
import time
import threading

try:
    import serial
    HAS_PYSERIAL = True
except ImportError:
    HAS_PYSERIAL = False


class ArduinoStepperSerialDriver:
    """
    Serial Driver Manager for communicating with Arduino Stepper Motor Controller
    """
    def __init__(self, port: str = '/dev/ttyACM0', baudrate: int = 115200, logger=None):
        self.port = port
        self.baudrate = baudrate
        self.logger = logger
        self.ser = None
        self.is_connected = False
        self.last_reconnect_attempt = 0.0
        self.last_sent_cmd = ""
        self.last_sent_time = 0.0

    def connect(self) -> bool:
        if not HAS_PYSERIAL:
            if self.logger:
                self.logger.warn("pyserial library is not installed (`pip install pyserial`). Serial driver inactive.")
            return False

        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=0.1)
            time.sleep(1.5)  # Allow Arduino DTR reset / reboot delay
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()
            
            # Send initial ping / handshake command
            self.ser.write(b"PING\n")
            time.sleep(0.05)
            response = self.ser.read_all().decode('utf-8', errors='ignore')
            
            self.is_connected = True
            if self.logger:
                self.logger.info(f"Successfully connected to Arduino Stepper Controller on {self.port} @ {self.baudrate} baud. Response: {response.strip()}")
            return True
        except Exception as e:
            self.is_connected = False
            if self.ser and self.ser.is_open:
                try:
                    self.ser.close()
                except Exception:
                    pass
            self.ser = None
            if self.logger:
                self.logger.warn(f"Failed to open USB serial port {self.port}: {e}")
            return False

    def auto_reconnect(self):
        now = time.time()
        if not self.is_connected and (now - self.last_reconnect_attempt > 3.0):
            self.last_reconnect_attempt = now
            self.connect()

    def send_motor_command(self, arm_dir: int, arm_freq: float, grip_dir: int, grip_freq: float, force_send: bool = False):
        """
        Sends formatted motion command to Arduino: M <arm_dir> <arm_freq> <grip_dir> <grip_freq>\n
        """
        if not self.is_connected or self.ser is None:
            self.auto_reconnect()
            if not self.is_connected:
                return

        cmd = f"M {int(arm_dir)} {int(arm_freq)} {int(grip_dir)} {int(grip_freq)}\n"
        now = time.time()

        # Send if command changed OR if 100ms passed (to maintain Arduino safety watchdog heartbeat)
        if force_send or cmd != self.last_sent_cmd or (now - self.last_sent_time > 0.1):
            try:
                self.ser.write(cmd.encode('utf-8'))
                self.last_sent_cmd = cmd
                self.last_sent_time = now
            except Exception as e:
                self.is_connected = False
                if self.logger:
                    self.logger.error(f"Serial communication error sending command: {e}")

    def send_stop(self):
        if self.is_connected and self.ser:
            try:
                self.ser.write(b"S\n")
            except Exception:
                pass

    def close(self):
        if self.ser and self.ser.is_open:
            try:
                self.send_stop()
                self.ser.close()
            except Exception:
                pass
        self.is_connected = False


class ArmStepperArduinoDriverNode(Node):
    def __init__(self):
        super().__init__('arm_stepper_arduino_driver')

        # Configuration Parameters
        self.declare_parameter('use_hardware_serial', True)
        self.declare_parameter('serial_port', '/dev/ttyACM0')
        self.declare_parameter('baud_rate', 115200)
        self.declare_parameter('dir_setup_us', 5.0)

        # DM556Y Driver #1: Arm Joint Stepper
        self.declare_parameter('arm_gear_ratio', 50.0)        # 50:1 gearbox on arm joint
        self.declare_parameter('arm_motor_steps', 1600)       # DM556Y microsteps
        self.declare_parameter('arm_invert_dir', False)
        self.declare_parameter('arm_max_velocity_rad_s', 0.5) # 0.5 rad/s (approx 30 deg/s)

        # DM556Y Driver #2: Gripper Stepper
        self.declare_parameter('gripper_lead_pitch', 0.008)   # 8mm lead screw pitch
        self.declare_parameter('gripper_motor_steps', 1600)   # DM556Y microsteps
        self.declare_parameter('gripper_invert_dir', False)
        self.declare_parameter('gripper_max_velocity_m_s', 0.05) # 0.05 m/s

        # Fetch Parameters
        self.use_hardware_serial = self.get_parameter('use_hardware_serial').value
        self.serial_port = self.get_parameter('serial_port').value
        self.baud_rate = self.get_parameter('baud_rate').value
        self.dir_setup_us = self.get_parameter('dir_setup_us').value

        self.gear_ratio = self.get_parameter('arm_gear_ratio').value
        self.arm_motor_steps = self.get_parameter('arm_motor_steps').value
        self.arm_invert_dir = self.get_parameter('arm_invert_dir').value
        self.arm_max_vel = self.get_parameter('arm_max_velocity_rad_s').value

        self.gripper_pitch = self.get_parameter('gripper_lead_pitch').value
        self.gripper_motor_steps = self.get_parameter('gripper_motor_steps').value
        self.gripper_invert_dir = self.get_parameter('gripper_invert_dir').value
        self.gripper_max_vel = self.get_parameter('gripper_max_velocity_m_s').value

        # Calculate Conversion Ratios & Step Pulse Frequencies
        self.arm_steps_per_rad = (self.gear_ratio * self.arm_motor_steps) / (2.0 * 3.14159265359)
        self.gripper_steps_per_m = self.gripper_motor_steps / self.gripper_pitch

        # Standard step frequency targets for high resolution motion
        self.arm_step_freq_hz = max(10000.0, self.arm_max_vel * self.arm_steps_per_rad)
        self.gripper_step_freq_hz = max(10000.0, self.gripper_max_vel * self.gripper_steps_per_m)

        # Initialize Arduino Serial Driver
        self.arduino_driver = ArduinoStepperSerialDriver(
            port=self.serial_port,
            baudrate=self.baud_rate,
            logger=self.get_logger()
        )

        self.hardware_initialized = False
        if self.use_hardware_serial:
            if HAS_PYSERIAL:
                if self.arduino_driver.connect():
                    self.hardware_initialized = True
                    self.get_logger().info(
                        f"Arduino Stepper Hardware Serial active on USB port {self.serial_port}.\n"
                        f"  Arm Step Freq Target: {self.arm_step_freq_hz/1000:.1f} kHz\n"
                        f"  Gripper Step Freq Target: {self.gripper_step_freq_hz/1000:.1f} kHz"
                    )
                else:
                    self.get_logger().warn(f"Could not connect to Arduino on {self.serial_port}. Running in simulation mode (will auto-retry connection).")
            else:
                self.get_logger().warn("pyserial package not installed. Running in simulation mode.")
        else:
            self.get_logger().info("use_hardware_serial set to False. Operating in simulation mode.")

        # Position tracking & target state
        self.current_arm_angle = 0.0
        self.target_arm_angle = 0.0
        self.current_arm_step = 0
        self.target_arm_step = 0

        # Gripper step tracking
        self.current_gripper_step = 0
        self.target_gripper_step = 0

        self.last_control_time = time.time()
        self.last_log_time = 0.0

        # ROS 2 Subscriptions & Publishers
        self.joint_sub = self.create_subscription(JointState, 'joint_states', self.joint_state_callback, 10)
        self.manual_vel_sub = self.create_subscription(Float32, '/arm_manual_vel', self.manual_vel_callback, 10)
        self.gripper_manual_vel_sub = self.create_subscription(Float32, '/gripper_manual_vel', self.gripper_manual_vel_callback, 10)
        self.gripper_state_pub = self.create_publisher(Float32, '/gripper_state', 10)
        self.gripper_target_sub = self.create_subscription(Float32, '/gripper_target', self.gripper_target_callback, 10)
        self.gripper_action_server = ActionServer(self, GripperCommand, 'gripper_command', self.execute_gripper_goal)

        self.manual_vel_input = 0.0
        self.last_manual_vel_time = 0.0
        self.gripper_manual_vel_input = 0.0
        self.last_gripper_manual_vel_time = 0.0

        # 50Hz high-speed control loop
        self.control_timer = self.create_timer(0.02, self.control_and_step_loop)

        self.get_logger().info('Arm & Gripper Arduino USB Serial Stepper Node Running.')
        self.get_logger().info(f'Arm conversion: {self.arm_steps_per_rad:.2f} steps/rad | Step Freq: {self.arm_step_freq_hz/1000:.1f} kHz')
        self.get_logger().info(f'Gripper conversion: {self.gripper_steps_per_m:.2f} steps/m | Step Freq: {self.gripper_step_freq_hz/1000:.1f} kHz')

        self.publish_gripper_state()

    def publish_gripper_state(self):
        state_msg = Float32()
        state_msg.data = float(self.current_gripper_step)
        self.gripper_state_pub.publish(state_msg)

    def manual_vel_callback(self, msg):
        self.manual_vel_input = msg.data
        self.last_manual_vel_time = time.time()

    def gripper_manual_vel_callback(self, msg):
        self.gripper_manual_vel_input = msg.data
        self.last_gripper_manual_vel_time = time.time()

    def joint_state_callback(self, msg):
        now_time = time.time()
        if (now_time - self.last_manual_vel_time < 0.4) and (abs(self.manual_vel_input) > 0.01):
            return
        if 'arm_joint' in msg.name:
            idx = msg.name.index('arm_joint')
            self.target_arm_angle = msg.position[idx]
            self.target_arm_step = int(self.target_arm_angle * self.arm_steps_per_rad)

    def gripper_target_callback(self, msg):
        self.target_gripper_step = int(msg.data)

    def control_and_step_loop(self):
        now_time = time.time()
        dt = now_time - self.last_control_time
        self.last_control_time = now_time

        arm_cmd_dir = 0
        arm_cmd_freq = 0.0

        grip_cmd_dir = 0
        grip_cmd_freq = 0.0

        # Check if direct manual arm velocity command is active (e.g. published by Dashboard Q/E)
        is_manual_active = (now_time - self.last_manual_vel_time < 0.4) and (abs(self.manual_vel_input) > 0.01)

        if is_manual_active:
            # Positive velocity = ARM UP (Forward), Negative velocity = ARM DOWN (Reverse)
            step_dir = 1 if self.manual_vel_input > 0 else -1
            if self.arm_invert_dir:
                step_dir = -step_dir

            angle_added = abs(self.manual_vel_input) * dt
            proposed_angle = self.current_arm_angle + (step_dir * angle_added)
            proposed_step = int(proposed_angle * self.arm_steps_per_rad)

            if proposed_step != self.current_arm_step:
                arm_cmd_dir = step_dir
                arm_cmd_freq = self.arm_step_freq_hz
            else:
                arm_cmd_dir = 0
                arm_cmd_freq = 0.0

            self.current_arm_step = proposed_step
            self.current_arm_angle = proposed_angle
            self.target_arm_step = self.current_arm_step
            self.target_arm_angle = self.current_arm_angle
        else:
            # Position tracking mode via /joint_states
            arm_step_diff = self.target_arm_step - self.current_arm_step
            if abs(arm_step_diff) > 2:
                step_dir = 1 if arm_step_diff > 0 else -1
                if self.arm_invert_dir:
                    step_dir = -step_dir

                arm_cmd_dir = step_dir
                arm_cmd_freq = self.arm_step_freq_hz

                max_steps_sec = self.arm_max_vel * self.arm_steps_per_rad
                steps_added = int(max_steps_sec * dt)
                steps_to_move = min(abs(arm_step_diff), max(1, steps_added))
                self.current_arm_step += (1 if arm_step_diff > 0 else -1) * steps_to_move
                self.current_arm_angle = self.current_arm_step / self.arm_steps_per_rad
            else:
                arm_cmd_dir = 0
                arm_cmd_freq = 0.0

        # Process Gripper Stepper Motor
        is_gripper_manual_active = (now_time - self.last_gripper_manual_vel_time < 0.4) and (abs(self.gripper_manual_vel_input) > 0.01)

        if is_gripper_manual_active:
            step_dir = 1 if self.gripper_manual_vel_input > 0 else -1
            if self.gripper_invert_dir:
                step_dir = -step_dir

            max_gripper_sec = self.gripper_max_vel * self.gripper_steps_per_m
            steps_added = int(max_gripper_sec * dt)
            steps_to_move = max(1, steps_added)

            self.current_gripper_step += step_dir * steps_to_move
            self.target_gripper_step = self.current_gripper_step

            grip_cmd_dir = step_dir
            grip_cmd_freq = self.gripper_step_freq_hz
            self.publish_gripper_state()
        else:
            gripper_step_diff = self.target_gripper_step - self.current_gripper_step
            if abs(gripper_step_diff) > 2:
                step_dir = 1 if gripper_step_diff > 0 else -1
                if self.gripper_invert_dir:
                    step_dir = -step_dir

                grip_cmd_dir = step_dir
                grip_cmd_freq = self.gripper_step_freq_hz

                max_gripper_sec = self.gripper_max_vel * self.gripper_steps_per_m
                steps_added = int(max_gripper_sec * dt)
                steps_to_move = min(abs(gripper_step_diff), max(1, steps_added))
                self.current_gripper_step += (1 if gripper_step_diff > 0 else -1) * steps_to_move
                self.publish_gripper_state()
            else:
                grip_cmd_dir = 0
                grip_cmd_freq = 0.0

        # Send command frame over USB Serial to Arduino
        if self.use_hardware_serial:
            self.arduino_driver.send_motor_command(
                arm_dir=arm_cmd_dir,
                arm_freq=arm_cmd_freq,
                grip_dir=grip_cmd_dir,
                grip_freq=grip_cmd_freq
            )

        # Throttled logging output (1 Hz)
        if now_time - self.last_log_time >= 1.0:
            self.last_log_time = now_time
            hw_str = f"ARDUINO SERIAL ({self.serial_port})" if self.arduino_driver.is_connected else "SIMULATION"
            self.get_logger().info(
                f'[{hw_str}] Arm: {self.current_arm_angle:.3f} rad ({self.current_arm_step}/{self.target_arm_step} steps) | '
                f'Gripper: {self.current_gripper_pos:.2f} ({self.current_gripper_step}/{self.target_gripper_step} steps)'
            )

    def execute_gripper_goal(self, goal_handle):
        target_pos = max(0.0, min(1.0, goal_handle.request.position))
        self.target_gripper_pos = target_pos
        finger_linear_pos = (1.0 - self.target_gripper_pos) * 0.2
        self.target_gripper_step = int(finger_linear_pos * self.gripper_steps_per_m)

        self.get_logger().info(f'Action Gripper Command received: moving to target pos {target_pos:.2f}')

        feedback_msg = GripperCommand.Feedback()
        rate = self.create_rate(10)

        while abs(self.current_gripper_pos - target_pos) > 0.01:
            feedback_msg.progress = float(self.current_gripper_pos)
            goal_handle.publish_feedback(feedback_msg)
            rate.sleep()

        goal_handle.succeed()
        result = GripperCommand.Result()
        result.success = True
        return result

    def destroy_node(self):
        if self.arduino_driver:
            self.arduino_driver.close()
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = ArmStepperArduinoDriverNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

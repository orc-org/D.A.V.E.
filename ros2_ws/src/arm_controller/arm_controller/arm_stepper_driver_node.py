import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer
from sensor_msgs.msg import JointState
from std_msgs.msg import Float32
from arm_interfaces.action import GripperCommand
import time
import threading

try:
    import Jetson.GPIO as GPIO
    HAS_JETSON_GPIO = True
except ImportError:
    HAS_JETSON_GPIO = False


class DM556YHardwareDriver:
    def __init__(self, step_pin: int, dir_pin: int, enable_pin: int = None, 
                 invert_dir: bool = False, active_low: bool = True,
                 step_freq_hz: float = 200000.0, dir_setup_us: float = 5.0):
        self.step_pin = step_pin
        self.dir_pin = dir_pin
        self.enable_pin = enable_pin
        self.invert_dir = invert_dir
        self.active_low = active_low
        self.step_freq_hz = step_freq_hz
        self.dir_setup_s = max(5.0e-6, dir_setup_us / 1e6)
        self.current_dir = None
        self.enabled = False
        self.hardware_active = False
        self.pwm = None
        self.is_pwm_running = False

    def init_gpio(self) -> bool:
        if not HAS_JETSON_GPIO:
            return False
        try:
            idle_state = GPIO.HIGH if self.active_low else GPIO.LOW
            GPIO.setup(self.step_pin, GPIO.OUT, initial=idle_state)
            GPIO.setup(self.dir_pin, GPIO.OUT, initial=idle_state)
            if self.enable_pin is not None:
                GPIO.setup(self.enable_pin, GPIO.OUT, initial=GPIO.HIGH if self.active_low else GPIO.LOW)
            
            # Set initial step pin output to LOW (Off / Idle state)
            GPIO.output(self.step_pin, GPIO.LOW)
            
            # Explicitly drive direction pin to solid 3.3V HIGH on startup
            GPIO.output(self.dir_pin, GPIO.HIGH)
            self.current_dir = True
            
            self.enabled = True
            self.hardware_active = True
            return True
        except Exception as e:
            print(f"DEBUG GPIO INIT ERROR on step={self.step_pin}, dir={self.dir_pin}, enable={self.enable_pin}: {e}")
            self.hardware_active = False
            return False

    def set_direction(self, forward: bool):
        direction_state = forward if not self.invert_dir else not forward
        if self.hardware_active:
            if self.active_low:
                GPIO.output(self.dir_pin, GPIO.LOW if direction_state else GPIO.HIGH)
            else:
                GPIO.output(self.dir_pin, GPIO.HIGH if direction_state else GPIO.LOW)
            if self.current_dir != direction_state:
                self.current_dir = direction_state
                time.sleep(self.dir_setup_s)

    def start_pwm(self, forward: bool, freq_hz: float = None):
        """Drives step_pin to digital HIGH to signal motion trigger to Arduino slave driver"""
        if not self.hardware_active:
            return
        
        self.set_direction(forward)
        
        if not self.is_pwm_running:
            try:
                # Set step pin output to digital HIGH (Motion Active)
                step_active_state = GPIO.LOW if self.active_low else GPIO.HIGH
                GPIO.output(self.step_pin, step_active_state)
                self.is_pwm_running = True
            except Exception:
                pass

    def stop_pwm(self):
        """Drives step_pin to digital LOW (0.0V DC) to stop Arduino step pulse generator"""
        if self.hardware_active and self.is_pwm_running:
            try:
                step_idle_state = GPIO.HIGH if self.active_low else GPIO.LOW
                GPIO.output(self.step_pin, step_idle_state)
            except Exception:
                pass
            self.is_pwm_running = False

    def set_enable(self, enable: bool):
        self.enabled = enable
        if self.hardware_active and self.enable_pin is not None:
            if self.active_low:
                GPIO.output(self.enable_pin, GPIO.HIGH if enable else GPIO.LOW)
            else:
                GPIO.output(self.enable_pin, GPIO.LOW if enable else GPIO.HIGH)


class ArmStepperDriverNode(Node):
    def __init__(self):
        super().__init__('arm_stepper_driver')

        # Configuration Parameters
        self.declare_parameter('use_hardware_gpio', True)
        self.declare_parameter('dir_setup_us', 5.0)

        # DM556Y Driver #1: Arm Joint Stepper
        self.declare_parameter('arm_gear_ratio', 50.0)        # 50:1 gearbox on arm joint
        self.declare_parameter('arm_motor_steps', 1600)       # DM556Y microsteps
        self.declare_parameter('arm_step_pin', 33)            # Jetson Board Pin 33 (STEP)
        self.declare_parameter('arm_dir_pin', 18)             # Jetson Board Pin 18 (DIR)
        self.declare_parameter('arm_enable_pin', 22)          # Jetson Board Pin 22 (ENABLE)
        self.declare_parameter('arm_invert_dir', False)
        self.declare_parameter('arm_active_low', False)       # False for standard 3.3V HIGH (UP) / 0V LOW (DOWN) logic
        self.declare_parameter('arm_max_velocity_rad_s', 0.5) # 0.5 rad/s (approx 30 deg/s for smooth motion)

        # DM556Y Driver #2: Gripper Stepper
        self.declare_parameter('gripper_lead_pitch', 0.008)   # 8mm lead screw pitch
        self.declare_parameter('gripper_motor_steps', 1600)   # DM556Y microsteps
        self.declare_parameter('gripper_step_pin', 32)       # Jetson Board Pin 32 (STEP)
        self.declare_parameter('gripper_dir_pin', 13)        # Jetson Board Pin 13 (DIR)
        self.declare_parameter('gripper_enable_pin', 16)     # Jetson Board Pin 16 (ENABLE)
        self.declare_parameter('gripper_invert_dir', False)
        self.declare_parameter('gripper_active_low', False)      # False for standard 3.3V HIGH (OPEN) / 0V LOW (CLOSE) logic
        self.declare_parameter('gripper_max_velocity_m_s', 0.05) # 0.05 m/s

        # Fetch Parameters
        self.use_hardware_gpio = self.get_parameter('use_hardware_gpio').value
        self.dir_setup_us = self.get_parameter('dir_setup_us').value

        self.gear_ratio = self.get_parameter('arm_gear_ratio').value
        self.arm_motor_steps = self.get_parameter('arm_motor_steps').value
        self.arm_step_pin = self.get_parameter('arm_step_pin').value
        self.arm_dir_pin = self.get_parameter('arm_dir_pin').value
        self.arm_enable_pin = self.get_parameter('arm_enable_pin').value
        self.arm_invert_dir = self.get_parameter('arm_invert_dir').value
        self.arm_active_low = self.get_parameter('arm_active_low').value
        self.arm_max_vel = self.get_parameter('arm_max_velocity_rad_s').value

        self.gripper_pitch = self.get_parameter('gripper_lead_pitch').value
        self.gripper_motor_steps = self.get_parameter('gripper_motor_steps').value
        self.gripper_step_pin = self.get_parameter('gripper_step_pin').value
        self.gripper_dir_pin = self.get_parameter('gripper_dir_pin').value
        self.gripper_enable_pin = self.get_parameter('gripper_enable_pin').value
        self.gripper_invert_dir = self.get_parameter('gripper_invert_dir').value
        self.gripper_active_low = self.get_parameter('gripper_active_low').value
        self.gripper_max_vel = self.get_parameter('gripper_max_velocity_m_s').value

        # Calculate Conversion Ratios & Physical Pulse Frequencies
        self.arm_steps_per_rad = (self.gear_ratio * self.arm_motor_steps) / (2.0 * 3.14159265359)
        self.gripper_steps_per_m = self.gripper_motor_steps / self.gripper_pitch

        # Ensure high pulse frequency (100 kHz minimum) so motor turns reliably
        self.arm_step_freq_hz = max(100000.0, self.arm_max_vel * self.arm_steps_per_rad)
        self.gripper_step_freq_hz = max(100000.0, self.gripper_max_vel * self.gripper_steps_per_m)

        # Initialize Hardware Drivers for DM556Y #1 and #2
        self.arm_hw = DM556YHardwareDriver(
            step_pin=self.arm_step_pin,
            dir_pin=self.arm_dir_pin,
            enable_pin=self.arm_enable_pin,
            invert_dir=self.arm_invert_dir,
            active_low=self.arm_active_low,
            step_freq_hz=self.arm_step_freq_hz,
            dir_setup_us=self.dir_setup_us
        )
        self.gripper_hw = DM556YHardwareDriver(
            step_pin=self.gripper_step_pin,
            dir_pin=self.gripper_dir_pin,
            enable_pin=self.gripper_enable_pin,
            invert_dir=self.gripper_invert_dir,
            active_low=self.gripper_active_low,
            step_freq_hz=self.gripper_step_freq_hz,
            dir_setup_us=self.dir_setup_us
        )

        self.hardware_initialized = False
        if self.use_hardware_gpio and HAS_JETSON_GPIO:
            try:
                GPIO.setwarnings(False)
                GPIO.setmode(GPIO.BOARD)
                arm_ok = self.arm_hw.init_gpio()
                gripper_ok = self.gripper_hw.init_gpio()
                if arm_ok and gripper_ok:
                    self.hardware_initialized = True
                    self.get_logger().info(
                        f'Jetson Orin Nano Hardware PWM GPIO initialized for 2x DM556Y Stepper Drivers.\n'
                        f'  Driver #1 (Arm): STEP=Pin {self.arm_step_pin} ({self.arm_step_freq_hz/1000:.1f}kHz PWM), DIR=Pin {self.arm_dir_pin}\n'
                        f'  Driver #2 (Gripper): STEP=Pin {self.gripper_step_pin} ({self.gripper_step_freq_hz/1000:.1f}kHz PWM), DIR=Pin {self.gripper_dir_pin}'
                    )
                else:
                    self.get_logger().warn('Failed to initialize GPIO pins on Jetson Orin Nano. Operating in simulation mode.')
            except Exception as e:
                self.get_logger().warn(f'GPIO setup exception ({e}). Falling back to simulation mode.')
        else:
            if not HAS_JETSON_GPIO:
                self.get_logger().warn('Jetson.GPIO library unavailable. Operating in simulation mode.')
            else:
                self.get_logger().info('use_hardware_gpio set to False. Operating in simulation mode.')

        # Position tracking & target state
        self.current_arm_angle = 0.0
        self.target_arm_angle = 0.0
        self.current_arm_step = 0
        self.target_arm_step = 0

        # Assuming the gripper starts fully OPEN (0.0)
        self.target_gripper_pos = 0.0   # 0.0 = open, 1.0 = closed
        self.current_gripper_pos = 0.0
        
        # 0.0 (open) mathematically equals 40,000 steps (0.2m * 200,000 steps/m)
        # 1.0 (closed) mathematically equals 0 steps
        self.current_gripper_step = int(0.2 * self.gripper_steps_per_m)
        self.target_gripper_step = self.current_gripper_step

        self.last_control_time = time.time()
        self.last_log_time = 0.0

        # ROS 2 Subscriptions & Publishers
        self.joint_sub = self.create_subscription(JointState, 'joint_states', self.joint_state_callback, 10)
        self.manual_vel_sub = self.create_subscription(Float32, '/arm_manual_vel', self.manual_vel_callback, 10)
        self.gripper_state_pub = self.create_publisher(Float32, '/gripper_state', 10)
        self.gripper_target_sub = self.create_subscription(Float32, '/gripper_target', self.gripper_target_callback, 10)
        self.gripper_action_server = ActionServer(self, GripperCommand, 'gripper_command', self.execute_gripper_goal)

        self.manual_vel_input = 0.0
        self.last_manual_vel_time = 0.0

        # 50Hz high-speed control loop
        self.control_timer = self.create_timer(0.02, self.control_and_step_loop)

        self.get_logger().info('Arm & Gripper DM556Y High-Speed PWM Stepper Node Running.')
        self.get_logger().info(f'Arm conversion: {self.arm_steps_per_rad:.2f} steps/rad | Step Freq: {self.arm_step_freq_hz/1000:.1f} kHz')
        self.get_logger().info(f'Gripper conversion: {self.gripper_steps_per_m:.2f} steps/m | Step Freq: {self.gripper_step_freq_hz/1000:.1f} kHz')

        self.publish_gripper_state()

    def publish_gripper_state(self):
        state_msg = Float32()
        state_msg.data = float(self.current_gripper_pos)
        self.gripper_state_pub.publish(state_msg)

    def manual_vel_callback(self, msg):
        self.manual_vel_input = msg.data
        self.last_manual_vel_time = time.time()

    def joint_state_callback(self, msg):
        if 'arm_joint' in msg.name:
            idx = msg.name.index('arm_joint')
            self.target_arm_angle = msg.position[idx]
            self.target_arm_step = int(self.target_arm_angle * self.arm_steps_per_rad)
            # DEBUG LOG
            self.get_logger().info(f"DEBUG: joint_state_callback updated target_arm_step to {self.target_arm_step}")

    def gripper_target_callback(self, msg):
        self.target_gripper_pos = max(0.0, min(1.0, msg.data))
        finger_linear_pos = (1.0 - self.target_gripper_pos) * 0.2
        self.target_gripper_step = int(finger_linear_pos * self.gripper_steps_per_m)

    def control_and_step_loop(self):
        now_time = time.time()
        dt = now_time - self.last_control_time
        self.last_control_time = now_time

        # Check if direct manual arm velocity command is active (published by Dashboard Q/E)
        is_manual_active = (now_time - self.last_manual_vel_time < 0.4) and (abs(self.manual_vel_input) > 0.01)

        if is_manual_active:
            # Positive velocity = ARM UP (Forward), Negative velocity = ARM DOWN (Reverse)
            step_dir = 1 if self.manual_vel_input > 0 else -1
            
            # Step position accumulation during continuous motion (Match dashboard speed perfectly)
            # Mathematically integrate the float angle first to completely eliminate int truncation drift over time
            angle_added = abs(self.manual_vel_input) * dt
            proposed_angle = self.current_arm_angle + (step_dir * angle_added)
            
            # Clamp to +/- 90 degrees (1.570796 rad) to match the dashboard limits
            proposed_angle = max(-1.57079632679, min(1.57079632679, proposed_angle))
            proposed_step = int(proposed_angle * self.arm_steps_per_rad)
            
            # Only start PWM if there's actually a physical step needed
            if proposed_step != self.current_arm_step:
                self.arm_hw.start_pwm(forward=(step_dir > 0), freq_hz=self.arm_step_freq_hz)
            else:
                self.arm_hw.stop_pwm()
                
            self.current_arm_step = proposed_step
            self.current_arm_angle = proposed_angle
            self.target_arm_step = self.current_arm_step
            self.target_arm_angle = self.current_arm_angle
        else:
            # Position tracking mode via /joint_states
            arm_step_diff = self.target_arm_step - self.current_arm_step
            if abs(arm_step_diff) > 2:
                step_dir = 1 if arm_step_diff > 0 else -1
                self.arm_hw.start_pwm(forward=(step_dir > 0), freq_hz=self.arm_step_freq_hz)
                
                max_steps_sec = self.arm_max_vel * self.arm_steps_per_rad
                steps_added = int(max_steps_sec * dt)
                steps_to_move = min(abs(arm_step_diff), max(1, steps_added))
                self.current_arm_step += step_dir * steps_to_move
                self.current_arm_angle = self.current_arm_step / self.arm_steps_per_rad
            else:
                self.arm_hw.stop_pwm()

        # 2. Process Gripper Stepper Motor (DM556Y #2 - 50 kHz PWM)
        gripper_step_diff = self.target_gripper_step - self.current_gripper_step
        if abs(gripper_step_diff) > 2:
            step_dir = 1 if gripper_step_diff > 0 else -1
            self.gripper_hw.start_pwm(forward=(step_dir > 0), freq_hz=self.gripper_step_freq_hz)
            
            max_gripper_sec = self.gripper_max_vel * self.gripper_steps_per_m
            steps_added = int(max_gripper_sec * dt)
            steps_to_move = min(abs(gripper_step_diff), max(1, steps_added))
            self.current_gripper_step += step_dir * steps_to_move
            
            cur_linear = self.current_gripper_step / self.gripper_steps_per_m
            self.current_gripper_pos = max(0.0, min(1.0, 1.0 - (cur_linear / 0.2)))
            self.publish_gripper_state()
        else:
            self.gripper_hw.stop_pwm()

        # Throttled logging output
        if now_time - self.last_log_time >= 1.0:
            self.last_log_time = now_time
            hw_str = f"HARDWARE PWM ({self.arm_step_freq_hz/1000:.0f}kHz)" if self.hardware_initialized else "SIMULATION"
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
        if self.hardware_initialized:
            try:
                self.arm_hw.stop_pwm()
                self.gripper_hw.stop_pwm()
                self.arm_hw.set_enable(False)
                self.gripper_hw.set_enable(False)
                GPIO.cleanup()
            except Exception:
                pass

def main(args=None):
    rclpy.init(args=args)
    node = ArmStepperDriverNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

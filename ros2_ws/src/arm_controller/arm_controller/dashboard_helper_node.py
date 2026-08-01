import rclpy
from rclpy.node import Node
from std_srvs.srv import SetBool
from std_msgs.msg import String
import subprocess
import json
import os
import signal

class DashboardHelperNode(Node):
    def __init__(self):
        super().__init__('dashboard_helper')
        
        # define processes: key -> display_name, cmd_args, pgrep_pattern
        self.processes = {
            "tf_republisher": {
                "name": "TF Web Republisher",
                "cmd": ["ros2", "run", "tf2_web_republisher", "tf2_web_republisher_node"],
                "pattern": "tf2_web_republisher_node",
                "proc": None
            },
            "state_publisher": {
                "name": "Robot State Publisher",
                "cmd": ["ros2", "launch", "rover_description", "view_robot.launch.py", "use_rviz:=false"],
                "pattern": "view_robot.launch.py",
                "proc": None
            },
            "motion_coordinator": {
                "name": "Motion Coordinator",
                "cmd": ["ros2", "run", "arm_controller", "motion_coordinator"],
                "pattern": "motion_coordinator",
                "proc": None
            },
            "drive_node": {
                "name": "Drive Node (Manual/Joystick)",
                "cmd": ["ros2", "run", "drive_package", "drive_node"],
                "pattern": "drive_package/drive_node",
                "proc": None
            },
            "vesc_driver": {
                "name": "VESC BLDC Wheel Driver",
                "cmd": ["ros2", "run", "drive_package", "vesc_can_driver_node", "--ros-args", "-p", "fl_can_id:=53", "-p", "fr_can_id:=44", "-p", "rl_can_id:=48", "-p", "rr_can_id:=7"],
                "pattern": "vesc_can_driver_node",
                "proc": None
            },
            "arm_stepper_driver": {
                "name": "Arm & Gripper Steppers (DM556Y)",
                "cmd": ["ros2", "run", "arm_controller", "arm_stepper_driver"],
                "pattern": "arm_stepper_driver",
                "proc": None
            },
            "motion_coordinator": {
                "name": "Arm Motion Coordinator",
                "cmd": ["ros2", "run", "arm_controller", "motion_coordinator"],
                "pattern": "motion_coordinator",
                "proc": None
            },
            "stream_cam_0": {
                "name": "Camera 0 Streamer (CSIPort0)",
                "cmd": ["python3", "/home/orc/D.A.V.E./ros2_ws/src/rover_video_streamer/rover_video_streamer/video_streamer.py", "--camera-type", "csi", "--device", "0", "--host", "192.168.1.87", "--port", "5000"],
                "pattern": "video_streamer.py.*--device 0",
                "proc": None
            },
            "stream_cam_1": {
                "name": "Camera 1 Streamer (CSIPort1)",
                "cmd": ["python3", "/home/orc/D.A.V.E./ros2_ws/src/rover_video_streamer/rover_video_streamer/video_streamer.py", "--camera-type", "csi", "--device", "1", "--host", "192.168.1.87", "--port", "5002"],
                "pattern": "video_streamer.py.*--device 1",
                "proc": None
            },
            "gps_bridge": {
                "name": "GPS Serial Bridge",
                "cmd": ["ros2", "run", "navigation_system", "gps_bridge_node.py"],
                "pattern": "gps_bridge_node",
                "proc": None
            },
            "nav_node": {
                "name": "Navigation Controller (NAVnode)",
                "cmd": ["ros2", "run", "navigation_system", "NAVnode"],
                "pattern": "NAVnode",
                "proc": None
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 10ca50e (ai slop dashboard part something)
            },
            "imu_telemetry": {
                "name": "IMU Telemetry Node",
                "cmd": ["ros2", "run", "imu", "telemetry"],
                "pattern": "imu.*telemetry",
                "proc": None
            },
            "imu_kalman_filter": {
                "name": "IMU Kalman Filter",
                "cmd": ["ros2", "run", "imu", "kalman_filter"],
                "pattern": "imu.*kalman_filter",
                "proc": None
            },
            "imu_launch": {
                "name": "IMU System Launch (HW)",
                "cmd": ["ros2", "launch", "imu", "imu.launch.py", "sim_mode:=false"],
                "pattern": "imu.launch.py",
                "proc": None
            },
            "rover_ekf": {
                "name": "Rover EKF Sensor Fusion (Wheel+IMU+GPS)",
                "cmd": ["ros2", "run", "rover_ekf", "ekf_node"],
                "pattern": "rover_ekf.*ekf_node",
                "proc": None
<<<<<<< HEAD
            },
            "morse_recorder": {
                "name": "Morse Code Recorder (UDP/Camera)",
                "cmd": ["ros2", "run", "enigma_machine", "morse_recorder.py"],
                "pattern": "morse_recorder.py",
                "proc": None
            },
            "enigma_node": {
                "name": "Enigma Machine Decoder/Encoder",
                "cmd": ["ros2", "run", "enigma_machine", "enigma_node"],
                "pattern": "enigma_node",
                "proc": None
=======
>>>>>>> 9bfcf53 (gps slop)
=======
>>>>>>> 10ca50e (ai slop dashboard part something)
            }
        }

        # Create Services
        for key in self.processes:
            service_name = f'/process_manager/toggle_{key}'
            # Use default argument capture in lambda
            self.create_service(
                SetBool,
                service_name,
                lambda req, resp, k=key: self.toggle_proc(k, req, resp)
            )
            self.get_logger().info(f'Created service: {service_name}')

        # Status Publisher
        self.status_pub = self.create_publisher(String, '/process_manager/status', 10)
        self.timer = self.create_timer(1.0, self.publish_status)

        self.get_logger().info('Dashboard Helper Node successfully initialized.')

    def is_running(self, pattern):
        try:
            # Check using pgrep. We ignore our own process name.
            res = subprocess.run(["pgrep", "-f", pattern], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if res.returncode == 0:
                pids = res.stdout.decode().strip().split()
                own_pid = str(os.getpid())
                valid_pids = []
                for pid in pids:
                    if pid == own_pid:
                        continue
                    try:
                        with open(f"/proc/{pid}/comm", "r") as f:
                            comm = f.read().strip()
                        if comm in ["pgrep", "pkill"]:
                            continue
                        with open(f"/proc/{pid}/cmdline", "r") as f:
                            cmdline = f.read().replace('\x00', ' ')
                        if "service" in cmdline or "toggle_" in cmdline:
                            continue
                    except Exception:
                        pass
                    valid_pids.append(pid)
                return len(valid_pids) > 0
            return False
        except Exception:
            return False

    def toggle_proc(self, key, request, response):
        proc_info = self.processes.get(key)
        if not proc_info:
            response.success = False
            response.message = f"Process key {key} not found."
            return response

        should_start = request.data
        pattern = proc_info["pattern"]
        cmd = proc_info["cmd"]

        currently_running = self.is_running(pattern)

        if should_start:
            if currently_running:
                response.success = True
                response.message = f"{proc_info['name']} is already running."
            else:
                try:
                    self.get_logger().info(f"Starting process: {proc_info['name']} via command: {cmd}")
                    cmd_str = " ".join(cmd)
                    full_cmd = [
                        "bash", "-c",
                        f"source /opt/ros/humble/setup.bash 2>/dev/null; source /home/orc/D.A.V.E./ros2_ws/install/setup.bash 2>/dev/null; export ROS_DOMAIN_ID=1; exec {cmd_str}"
                    ]
                    log_file = open(f"/tmp/dashboard_proc_{key}.log", "w")
                    proc = subprocess.Popen(
                        full_cmd,
                        stdout=log_file,
                        stderr=log_file,
                        preexec_fn=os.setsid
                    )
                    proc_info["proc"] = proc
                    response.success = True
                    response.message = f"Started {proc_info['name']} successfully."
                except Exception as e:
                    response.success = False
                    response.message = f"Failed to start {proc_info['name']}: {str(e)}"
        else:
            if not currently_running:
                response.success = True
                response.message = f"{proc_info['name']} is already stopped."
            else:
                try:
                    self.get_logger().info(f"Stopping process: {proc_info['name']} (pattern: {pattern})")
                    # Terminate subprocess if we launched it
                    if proc_info["proc"]:
                        try:
                            os.killpg(os.getpgid(proc_info["proc"].pid), signal.SIGTERM)
                            proc_info["proc"].wait(timeout=1.0)
                        except Exception:
                            pass
                        proc_info["proc"] = None
                    
                    # Also force kill using pkill to clean up any externally launched ones
                    subprocess.run(["pkill", "-f", pattern])
                    response.success = True
                    response.message = f"Stopped {proc_info['name']} successfully."
                except Exception as e:
                    response.success = False
                    response.message = f"Error stopping {proc_info['name']}: {str(e)}"

        return response

    def publish_status(self):
        if not rclpy.ok():
            return
        try:
            status_dict = {}
            for key, info in self.processes.items():
                status_dict[key] = {
                    "name": info["name"],
                    "running": self.is_running(info["pattern"])
                }
            
            msg = String()
            msg.data = json.dumps(status_dict)
            if rclpy.ok():
                self.status_pub.publish(msg)
        except Exception as e:
            pass

    def destroy_node(self):
        # Clean up any child processes on exit
        for key, info in self.processes.items():
            if info["proc"]:
                try:
                    os.killpg(os.getpgid(info["proc"].pid), signal.SIGTERM)
                except Exception:
                    pass
        super().destroy_node()

def main(args=None):
    rclpy.init(args=args)
    node = DashboardHelperNode()
    try:
        rclpy.spin(node)
    except (KeyboardInterrupt, BaseException):
        pass
    finally:
        try:
            node.destroy_node()
        except Exception:
            pass
        if rclpy.ok():
            rclpy.shutdown()

if __name__ == '__main__':
    main()

# Drive Package & Jetson CAN Configuration

This package contains control nodes for the rover's wheels (both stepper-based and VESC-based), including a native SocketCAN driver for direct CAN-bus motor control from the Jetson Orin Nano.

---

## 1. Manual CAN Port Setup (Command Cheat Sheet)

If you haven't automated the CAN interface configuration yet, run these commands sequentially to configure the Jetson Orin Nano's J17 CAN header, load kernel modules, and set the bitrate:

### Step A: Configure Pinmux Registers (Temporary until reboot)
This sets the J17 pins to special function IO (SFIO) mode for the CAN0 controller:
```bash
sudo apt-get install busybox
sudo busybox devmem 0x0c303018 w 0xc458  # J17 Pin 1 to CAN0 RX
sudo busybox devmem 0x0c303010 w 0xc400  # J17 Pin 2 to CAN0 TX
```

### Step B: Load CAN Modules & Bring Up Link
This loads the driver interfaces and initializes the network socket at 500,000 baud (500k):
```bash
sudo modprobe can
sudo modprobe can_raw
sudo modprobe mttcan

sudo ip link set can0 type can bitrate 500000
sudo ip link set up can0
```

try this next time:

    sudo modprobe can
    sudo modprobe can_raw
    sudo modprobe mttcan

    # 1. Set CAN interface type, bitrate, and auto-restart
    sudo ip link set can0 type can bitrate 500000 restart-ms 100

    # 2. Set transmit queue length
    sudo ip link set can0 txqueuelen 1000

    # 3. Bring the interface up
    sudo ip link set up can0

### Step C: Verify the Link Status
Check that the interface is running and has no electrical error states:
```bash
ip -details -statistics link show can0
```

---

## 2. CAN Message Formats & Message Explanations

VESC uses 29-bit Extended CAN IDs. Every frame sent to or from the VESC encodes two values:
*   **Command ID (Packet ID):** Indicates what action to take (e.g. Set Duty, Set Current, Set RPM).
*   **Controller ID (VESC ID):** The target or source VESC address (0 to 255).

### CAN ID Composition formula:
$$\text{Hex CAN ID} = (\text{Command ID} \ll 8) \mid \text{VESC ID}$$

---

### Command Mapping & Payload Explanations

#### 1. Command `0` : Set Duty Cycle (`CAN_PACKET_SET_DUTY`)
Used to control motor power percentage. 
*   **Payload Format:** 4-byte signed big-endian integer.
*   **Scale:** Value is multiplied by $100,000$.
*   **Calculation:** For $0.5$ (50%) duty cycle, payload $= 0.5 \times 100,000 = 50,000$ (Hex: `0x0000C350`).
*   **Example (VESC ID 124 / Hex `7c`):**
    ```bash
    # Set 50% duty cycle to VESC 124
    cansend can0 0000007c#0000C350
    ```

#### 2. Command `1` : Set Current (`CAN_PACKET_SET_CURRENT`)
Used to command torque / target current in Amps.
*   **Payload Format:** 4-byte signed big-endian integer.
*   **Scale:** Value is in milliamperes (mA) (multiplied by $1000$).
*   **Calculation:** For $10\text{A}$, payload $= 10 \times 1000 = 10,000$ (Hex: `0x00002710`).
*   **Example (VESC ID 124 / Hex `7c`):**
    ```bash
    # Set 10 Amps current target to VESC 124
    cansend can0 0000017c#00002710
    ```
    *(Note: CAN ID is `(1 << 8) | 124` = `0x0000017C`)*

#### 3. Command `3` : Set RPM (`CAN_PACKET_SET_RPM`)
Used to command speed / target ERPM (Electrical RPM).
*   **Payload Format:** 4-byte signed big-endian integer.
*   **Scale:** 1-to-1 representation of ERPM.
*   **Example (VESC ID 124 / Hex `7c`):**
    ```bash
    # Set 3000 ERPM target to VESC 124 (Hex value 3000 = 0x00000BB8)
    cansend can0 0000037c#00000BB8
    ```
    *(Note: CAN ID is `(3 << 8) | 124` = `0x0000037C`)*

#### 4. Command `9` : VESC Telemetry Status (`CAN_PACKET_STATUS`)
The VESC broadcasts status frames regularly (e.g., at 50Hz) to report telemetry back to the Jetson.
*   **CAN ID:** `0x0000097C` for VESC ID 124.
*   **Payload:** Includes motor RPM (bytes 0-3) and motor current (bytes 4-5).
*   **Monitoring Command:**
    ```bash
    candump can0
    ```

---

## 3. Running the ROS 2 Native CAN Node

The package includes the `vesc_can_driver_node` executable to bridge topic commands to SocketCAN. 

### Launching the node:
```bash
# Source setup files
source /home/orc/D.A.V.E./ros2_ws/install/setup.bash

# Run driver node overriding the CAN ID parameter to match your target VESC
ros2 run drive_package vesc_can_driver_node --ros-args -p fl_can_id:=124 -p control_mode:=duty
```
Once running, you can publish setpoints (between `-1.0` and `1.0`) on the steering topic to drive the motor:
```bash
ros2 topic pub --once /motor/front_left std_msgs/msg/Float32 "{data: 0.5}"
```





VESC_ID="00000035"
    INTERFACE="can0"
    
    echo "Ramping up to 50%..."
    for duty in $(seq 0 5000 50000); do
      hex_val=$(printf "%08X" $duty)
      cansend $INTERFACE ${VESC_ID}#${hex_val}
      sleep 0.1
    done
    
    echo "Holding 50% for 1 second..."
    for i in $(seq 1 80); do
      cansend $INTERFACE ${VESC_ID}#0000C350
      sleep 0.1
    done

    echo "Ramping down to 0%..."
    for duty in $(seq 50000 -5000 0); do
      hex_val=$(printf "%08X" $duty)
      cansend $INTERFACE ${VESC_ID}#${hex_val}
      sleep 0.1
    done

    # Final safety stop frame
    cansend $INTERFACE ${VESC_ID}#00000000
    echo "Done!"

     ### Diagnostic Quick-Reference Table

   Stage                                                       | Command                                                    | What to look for
  -------------------------------------------------------------|------------------------------------------------------------|------------------------------------------------------------
   1. SocketCAN                                                | ip -details link show can0                                 | state ERROR-ACTIVE, bitrate 500000
   2. Dashboard (Keyboard)                                     | ros2 topic echo /cmd_vel                                   | linear.x: 0.8 when W pressed
   3. Drive Mixer                                              | ros2 topic echo /motor/front_left                          | data: 0.8
   4. Driver Node Logs                                         | tail -f /tmp/dashboard_helper.log                          | cmd=0.800 | send_val=76000
   5. Physical CAN Bus                                         | candump can0                                               | ID 0000007C payload 00 01 28 E0
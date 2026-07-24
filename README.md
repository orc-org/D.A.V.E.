# D.A.V.E.
Introducing: D.A.V.E.! Okanagan Rover Craft's First Rover developed using the ROS2 Humble Hawksbill Framework! :) :)

---

## Base Station Remote Control Setup

This guide explains how to set up native ROS 2 network discovery between your base station laptop and the Jetson Orin Nano, allowing you to monitor topics, call services, and run commands like teleop directly from your laptop.

### 1. Environment Configuration

To allow ROS 2 nodes on separate machines to discover each other, they must share the same `ROS_DOMAIN_ID` and have localhost-only restrictions disabled.

Add the following lines to the end of `~/.bashrc` (or `~/.zshrc`) on **both** the Jetson and your laptop:

```bash
# set the same domain id on both machines to group them on the network (0-101)
export ROS_DOMAIN_ID=30

# enable ros 2 to broadcast outside of localhost
export ROS_LOCALHOST_ONLY=0
```

After editing, reload the terminal:
```bash
source ~/.bashrc
```

### 2. Firewall Configuration

DDS uses a specific port range (starting at `7400`) for discovery and data. Ensure these ports are open.

#### Linux (Jetson/Ubuntu Laptop)
If `ufw` is active, run this command to allow local network ROS 2 UDP traffic:
```bash
sudo ufw allow proto udp from 192.168.1.0/24 to any port 7400:7500
```
*(adjust the subnet `192.168.1.0/24` to match your local router's IP subnet)*

#### Windows (Base Station Laptop)
1. Open **Windows Defender Firewall with Advanced Security**.
2. Create a new **Inbound Rule** for **Port**.
3. Select **UDP** and specify ports **`7400-7500`**.
4. Choose **Allow the connection**.

### 3. Verify Discovery (Talker/Listener Test)

Test the connection using the standard ROS 2 demo nodes:

1. **On the Jetson:**
   ```bash
   ros2 run demo_nodes_cpp talker
   ```
2. **On the Laptop:**
   ```bash
   ros2 run demo_nodes_py listener
   ```

If discovery is successful, the laptop terminal will print the incrementing messages sent from the Jetson.

### 4. Remote Velocity Topic Test (Laptop -> Jetson)

This test verifies that command velocity messages published from your base station laptop are successfully received over the network by the Jetson.

#### Step A: Prepare the Jetson (Receiver)
1. Open a terminal on the Jetson.
2. Verify that your environment variables are active (`printenv | grep ROS`).
3. Run the topic echo command to listen for incoming drive messages:
   ```bash
   ros2 topic echo /cmd_vel
   ```
   *(This terminal will pause and wait for incoming messages).*

#### Step B: Send the Command from the Laptop (Sender)
1. Open a terminal on your base station laptop.
2. Ensure your ROS 2 environment is sourced and configured with the matching `ROS_DOMAIN_ID`.
3. Publish a test drive command to move the rover straight forward at `0.5 m/s`:
   ```bash
   ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.5, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}"
   ```

#### Step C: Verify Receipt
Check the Jetson's terminal window. If the network link is active, you should see the message printed immediately:
```yaml
linear:
  x: 0.5
  y: 0.0
  z: 0.0
angular:
  x: 0.0
  y: 0.0
  z: 0.0
---
```

Once this test succeeds, your network link is fully active! You can now run teleop keyboard nodes on your laptop to drive the rover physically.

---

## Troubleshooting: Unicast Fallback (For Routers Blocking Multicast)

Some Wi-Fi routers block UDP multicast for security, which stops auto-discovery. If the Talker/Listener test fails, you can force unicast by pointing directly to the IPs.

1. Create a `cyclonedds.xml` file on both machines:
   ```xml
   <?xml version="1.0" encoding="UTF-8" ?>
   <CycloneDDS xmlns="https://cdds.io/schemas/iceoryx" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="https://cdds.io/schemas/iceoryx https://cdds.io/schemas/iceoryx/cyclonedds.xsd">
       <Domain id="any">
           <General>
               <NetworkInterfaceAddress>AUTO</NetworkInterfaceAddress>
               <AllowMulticast>false</AllowMulticast>
           </General>
           <Discovery>
               <Peers>
                   <!-- Laptop IP -->
                   <Peer address="192.168.1.87"/>
                   <!-- Jetson IP -->
                   <Peer address="192.168.1.88"/>
               </Peers>
           </Discovery>
       </Domain>
   </CycloneDDS>
   ```

2. Add this export path to your `~/.bashrc`:
   ```bash
   export CYCLONEDDS_URI=file:///path/to/cyclonedds.xml
   ```

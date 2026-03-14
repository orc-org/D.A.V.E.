#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy
from visualization_msgs.msg import Marker, MarkerArray
from geometry_msgs.msg import Point
from ament_index_python.packages import get_package_share_directory
import os

#Hopefully these 2 make it so I dont need to keep making the rviz frame.
from tf2_ros import StaticTransformBroadcaster
from geometry_msgs.msg import TransformStamped

class input_viewer_class(Node):
    def __init__(self):
        super().__init__('input_viewer')

        self.sub = self.create_subscription(
            Joy,
            '/joy',
            self.joy_callback,
            10
        )

        self.pub = self.create_publisher(
            MarkerArray,
            '/joy_markers',
            10
        )

        self.get_logger().info('PS5 Input Viewer running (in RViz)')

        #Trying to stop making rviz frame each time
        self.tf_broadcaster = StaticTransformBroadcaster(self)
        self.publish_static_tf()

    def joy_callback(self, msg):
        markers = MarkerArray()
        mid = 0

        #Add background maybe (I hope)
        markers.markers.append(self.background_marker(mid))
        mid += 1

        
# LEFT STICK ORIGIN (axes 0,1)
    
        markers.markers.append(
            self.arrow_marker(
                #change the numbers for x,y origin
                mid, -1.91, -1.2,
                msg.axes[0], msg.axes[1],
                0.0, 0.0, 1.0
            )           
        )
        mid += 1

        
# RIGHT STICK ORIGIN (axes 2,3)
        
        markers.markers.append(
            self.arrow_marker(
                mid, 1.91, -1.2,
                msg.axes[2], msg.axes[3],
                1.0, 0.0, 0.0
            )
        )
        mid += 1

        
        # TRIGGERS (axes 4,5)

        #Normalizes the values to 0->1        
        l2 = (1.0 - msg.axes[4]) / 2.0
        r2 = (1.0 - msg.axes[5]) / 2.0

# Left Trigger Background
        markers.markers.append(self.trigger_background_marker(mid, -3.5, 1.9))
        mid += 1
#Left Trigger Origin
        markers.markers.append(self.bar_marker(mid, -3.5, 1.9, l2))
        mid += 1

# right trigger background
        markers.markers.append(self.trigger_background_marker(mid, 3.5, 1.9))
        mid += 1
#Right Trigger Origin
        markers.markers.append(self.bar_marker(mid,  3.5, 1.9, r2))
        mid += 1

 
# Button Origins
        button_positions = {
            0:  (3.72, -0.5),   # X (done)
            1:  (4.6, 0.325),   # Circle (Done)
            2:  (2.84, 0.325),  # Square (done)
            3:  (3.72, 1.15),   # Triangle (done)
            4:  (-2.8, 1.56),   # idk what this button is called(select?)
            5:  (0.0, -1.2),    # PS Button (probably dont hold this I think steam will open)
            6:  (2.8, 1.56),    # Start button?
            7:  (-1.91, -1.2),  # L3
            8:  (1.91, -1.2),   # R3
            9:  (-4.0, 2.3),    # LB
            10: (4.0, 2.3),     # RB
            11: (-3.73, 0.9),   # D-Pad Up (Done)
            12: (-3.73, -0.3),  # D-Pad Down (Done)
            13: (-4.35, 0.325), # D-Pad Left
            14: (-3.13, 0.325), # D-Pad Right
            15: (0.0, 1.5),     # Push Touchpad in (also dont touch this, joy node shits itself)
            16: (0.0, -1.8),    # I dont know what button this is (its the microphone one)
        }

        for i, pressed in enumerate(msg.buttons):
            if i in button_positions:
                x, y = button_positions[i]
                markers.markers.append(
                    self.button_marker(mid, x, y, pressed)        
                    )
                mid += 1

        self.pub.publish(markers)


    # MARKER HELPERS


    # The Function for the Stick Arrows
    def arrow_marker(self, mid, x_origin, y_origin, dx, dy, r, g, b):
        m = Marker()
        m.header.frame_id = 'joy_frame'
        m.header.stamp = self.get_clock().now().to_msg()
        m.ns = 'sticks'
        m.id = mid
        m.type = Marker.ARROW
        m.action = Marker.ADD

        #increase this to make the arrows longer
        arrow_length_scale = 1.5

        start = Point(x=x_origin, y=y_origin, z=0.0)
        end   = Point(x=x_origin - dx * arrow_length_scale, y=y_origin + dy*arrow_length_scale, z=0.0)

        m.points = [start, end]
        m.scale.x = 0.2
        m.scale.y = 0.4
        m.scale.z = 0.4

        m.color.r = r
        m.color.g = g
        m.color.b = b
        m.color.a = 1.0

        return m
    
    #Function for the trigger bars
    def bar_marker(self, mid, x, y, trigger_norm):
        m = Marker()
        m.header.frame_id = 'joy_frame'
        m.header.stamp = self.get_clock().now().to_msg()
        m.ns = 'triggers'
        m.id = mid
        m.type = Marker.CUBE
        m.action = Marker.ADD

        m.pose.position.x = x
        m.pose.position.y = y + trigger_norm / 2.0
        m.pose.position.z = 0.0

        m.scale.x = 0.3
        m.scale.y = max(0.01, trigger_norm)
        m.scale.z = 0.2

        m.color.r = 0.8
        m.color.g = 0.8
        m.color.b = 0.0
        m.color.a = 1.0

        return m
    
    #Should make a background for the trigger input
    def trigger_background_marker(self, mid, x, y):
        m = Marker()
        m.header.frame_id = 'joy_frame'
        m.header.stamp = self.get_clock().now().to_msg()
        m.ns = 'trigger_background'
        m.id = mid
        m.type = Marker.CUBE
        m.action = Marker.ADD

        m.pose.position.x = x
        m.pose.position.y = y + 0.5  # centers the background (since full height is 1.0)
        m.pose.position.z = 0.0

        #these should match the other function (except the y one and opacity)
        m.scale.x = 0.3  
        m.scale.y = 1.0  
        m.scale.z = 0.2  

        m.color.r = 0.8
        m.color.g = 0.8
        m.color.b = 0.0
        m.color.a = 0.2  # 20% opacity

        return m
    
    #Function for the buttons to light up
    def button_marker(self, mid, x, y, pressed):
        m = Marker()
        m.header.frame_id = 'joy_frame'
        m.header.stamp = self.get_clock().now().to_msg()
        m.ns = 'buttons'
        m.id = mid
        m.type = Marker.SPHERE
        m.action = Marker.ADD

        m.pose.position.x = x
        m.pose.position.y = y
        m.pose.position.z = 0.0

        m.scale.x = 0.5
        m.scale.y = 0.5
        m.scale.z = 0.5

        if pressed:
            m.color.r = 0.0
            m.color.g = 1.0
            m.color.b = 0.0
        else:
            m.color.r = 0.4
            m.color.g = 0.4
            m.color.b = 0.4

        m.color.a = 0.75
        return m
    
    #Hopefully function that makes background (im working on making transparency happen but for now we deal with the black rectangle)
    def background_marker(self, mid):
        m = Marker()
        m.header.frame_id = 'joy_frame'
        m.header.stamp = self.get_clock().now().to_msg()
        m.ns = 'background'
        m.id = mid
        m.type = Marker.MESH_RESOURCE
        m.action = Marker.ADD

        pkg_path = get_package_share_directory('drive_package')
        mesh_path = os.path.join(pkg_path, 'resource', 'controller_plane.dae')

        m.mesh_resource = f"file://{mesh_path}"
        m.mesh_use_embedded_materials = True

        m.pose.position.x = 0.0
        m.pose.position.y = -1.5
        m.pose.position.z = -0.05

        m.scale.x = 6.0
        m.scale.y = 4.0
        m.scale.z = 1.0

        m.color.r = 0.3
        m.color.b = 0.3
        m.color.g = 0.3

        #Changes opacity
        m.color.a = 1.0

        return m
    
    #hopefully makes the rviz frame
    def publish_static_tf(self):
        from geometry_msgs.msg import TransformStamped

        t = TransformStamped()

        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = 'world'
        t.child_frame_id = 'joy_frame'

        # No translation
        t.transform.translation.x = 0.0
        t.transform.translation.y = 0.0
        t.transform.translation.z = 0.0

        # Identity rotation
        t.transform.rotation.x = 0.0
        t.transform.rotation.y = 0.0
        t.transform.rotation.z = 0.0
        t.transform.rotation.w = 1.0

        self.tf_broadcaster.sendTransform(t)


def main():
    rclpy.init()
    node = input_viewer_class()
    rclpy.spin(node)
    rclpy.shutdown()


if __name__ == '__main__':
    main()

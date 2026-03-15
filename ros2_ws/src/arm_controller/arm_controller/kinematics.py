import numpy as np
import math
import matplotlib.pyplot as plt

# this class creates components that can be linked together to form a kinematic chain
# and also ways to move them.
class Component3D:
    def __init__(self, name, parent=None, offset_x=0.0, offset_y=0.0, offset_z=0.0, 
                 angle_x_deg=0.0, angle_y_deg=0.0, angle_z_deg = 0.0,
                 limit_x=None, limit_y=None, limit_z=None, fixed=False):

        self.name = name
        self.parent = parent
        self.children = [] # a list to hold any componenets attached to this one
        self.fixed = fixed # if True, IK will ignore this joint

        # the offsets define the physical distance from a parent's join to this joint
        # for example, if offset_X = 0.8 (y=0,z=0) that rover's arm is 0.8 (units?) long
        self.offset_x = offset_x
        self.offset_y = offset_y
        self.offset_z = offset_z

        # optional: set rotation limits [expects tuple (min_angle, max_angle), in degrees]
        self.limit_x = limit_x
        self.limit_y = limit_y
        self.limit_z = limit_z

        # this is how the component is bent/twisted relative to its parent
        self.angle_x = 0.0
        self.angle_y = 0.0
        self.angle_z = 0.0
        self.set_angles(angle_x_deg, angle_y_deg, angle_z_deg)

        
        # attaches itself to its parent
        if self.parent:
            self.parent.children.append(self)

    def set_angles(self, ax=None, ay=None, az=None):
        """Updates joint angles while enforcing physical limits."""
        if self.fixed: return # Fixed joints cannot have their angles changed
        
        # check X Axis
        if ax is not None:
            # if a limit exists, clamp the value. Otherwise, leave it alone.
            if self.limit_x: 
                ax = max(self.limit_x[0], min(ax, self.limit_x[1]))
            self.angle_x = math.radians(ax)
            
        # check Y Axis
        if ay is not None:
            if self.limit_y: 
                ay = max(self.limit_y[0], min(ay, self.limit_y[1]))
            self.angle_y = math.radians(ay)
            
        # check Z Axis
        if az is not None:
            if self.limit_z: 
                az = max(self.limit_z[0], min(az, self.limit_z[1]))
            self.angle_z = math.radians(az)

    # this is used for moving the base so we can figure out where the rover is relative the the arm
    def set_position(self, x=None, y=None, z=None):
        if x is not None: self.offset_x = x
        if y is not None: self.offset_y = y
        if z is not None: self.offset_z = z

    def get_local_transform(self):
        """ calculates the 4x4 Homogeneous Transformation Matrix for this specific joint,
            ignoring the rest of the chain"""
        
        # translation: moves the joints in space
        T = np.array([
            [1,0,0,self.offset_x],
            [0,1,0,self.offset_y],
            [0,0,1,self.offset_z],
            [0,0,0,1]
        ])

        cx, sx = math.cos(self.angle_x), math.sin(self.angle_x)
        cy, sy = math.cos(self.angle_y), math.sin(self.angle_y)
        cz, sz = math.cos(self.angle_z), math.sin(self.angle_z)
    
        # x-rotation: rotates around the x-axis (roll)
        Rx = np.array([
            [1,0,0,0],
            [0,cx,-sx,0],
            [0,sx,cx,0],
            [0,0,0,1]
        ])

        #y-rotation: rotates around the y-axis (Pitch)
        Ry = np.array([
            [ cy, 0, sy, 0],
            [  0, 1,  0, 0],
            [-sy, 0, cy, 0],
            [  0, 0,  0, 1]
        ])
        
        #z-rotation: rotates aruond the z-axis (Yaw)
        Rz = np.array([
            [cz, -sz, 0, 0],
            [sz,  cz, 0, 0],
            [ 0,   0, 1, 0],
            [ 0,   0, 0, 1]
        ])

        # combine the rotations by multiplying Z * Y * X
        rotation_matrix = np.dot(Rz, np.dot(Ry, Rx))
        # combine translation and rotation
        return np.dot(T, rotation_matrix)

    def get_global_transform(self):
        """ calculates where this joint is in the world by combining its local transform with
            whatever components it is attached to recursively"""
        local_tf = self.get_local_transform()
        if self.parent is None:
            return local_tf
        else:
            parent_global_tf = self.parent.get_global_transform()
            return np.dot(parent_global_tf, local_tf)

    def get_global_position(self):
        """ extracts X, Y, and Z from the 4x4 matrix (index 3 is the 4th column)."""
        global_tf = self.get_global_transform()
        x = global_tf[0, 3]
        y = global_tf[1, 3]
        z = global_tf[2, 3]
        return x, y, z

    def get_transform_relative_to(self, ancestor):
        """Calculates the transform of this component relative to a specific ancestor's frame."""
        local_tf = self.get_local_transform()
        if self.parent == ancestor or self.parent is None:
            return local_tf
        else:
            return np.dot(self.parent.get_transform_relative_to(ancestor), local_tf)

    def get_relative_position(self, ancestor):
        """Extracts X, Y, Z coordinates relative to an ancestor component."""
        rel_tf = self.get_transform_relative_to(ancestor)
        return rel_tf[0, 3], rel_tf[1, 3], rel_tf[2, 3]
    
    # this is basically just fake gradient descent, but it works(ish)!!! ignore that it is not particulary efficient :)
    def inverse_kinematics(self, end_effector, target_x, target_y, target_z, 
                           learning_rate=1.0, max_iterations=20, move_base=False,
                           max_angle_step=1.0, max_pos_step=0.1):
        """
        Calculates Inverse Kinematics using a hybrid of Gradient Descent (for the arm)
        and direct geometry (for navigation).
        
        Returns (total_linear_movement, total_angular_movement) if move_base is True.
        """
        total_linear = 0.0
        total_angular = 0.0
        
        # creates a list of all the joints in the chain
        def get_all_joints(component):
            joints = [component]
            for child in component.children:
                joints.extend(get_all_joints(child))
            return joints
            
        chain = get_all_joints(self)
        # wiggle is the amount of distance that we will move a joint to test if that movement brings us closer to our target
        wiggle = 0.1 
        
        for i in range(max_iterations):

            # find where the end effector is in 3d space.
            ex, ey, ez = end_effector.get_global_position()

            # distance from end effector to the target
            dist = math.sqrt((target_x - ex)**2 + (target_y - ey)**2 + (target_z - ez)**2)
            
            # this is whenever its close enough, so we stop computing
            if dist < 0.1: 
                break
                
            # iterate through every joint from the base to the end effector
            for joint in chain:
                if joint.fixed: continue # skip wiggling if the joint is fixed
                
                # print(f"Debugging IK: Wiggling {joint.name}") # optional debug print

                # save the current joint's current angles (in degrees)
                angles = [math.degrees(joint.angle_x), math.degrees(joint.angle_y), math.degrees(joint.angle_z)]
                
                # we will test rotating around each axis
                for axis in range(3):

                    # make a copy of each of the angles so we don't mess up the original ones yet
                    test_angles = angles.copy()
                    # wiggle the current axis forward by "wiggle" units
                    test_angles[axis] += wiggle 
                    # temporarily apply the wiggled angle to the joint
                    joint.set_angles(ax=test_angles[0], ay=test_angles[1], az=test_angles[2])

                    # calculate the new, temporary distance to the target
                    tex, tey, tez = end_effector.get_global_position()
                    new_dist = math.sqrt((target_x - tex)**2 + (target_y - tey)**2 + (target_z - tez)**2)
                    
                    # if new_dist is smaller than dist, the gradient is negative (YIPPEEE!!!!!)
                    # if new_dist is larger than dist, the gradient is positive (BAD!!!)
                    gradient = (new_dist - dist) / wiggle
                    
                    # multiply the learning rate by the gradient to determine how far to move (step)
                    step = learning_rate * gradient
                    step = max(-max_angle_step, min(max_angle_step, step)) # clamp the amount it can move per iteration
                    
                    # apply the step to the REAL angle
                    angles[axis] -= step
                    # set the new angles to the joint
                    joint.set_angles(ax=angles[0], ay=angles[1], az=angles[2])
                    
                    # update the baseline for the next angle test
                    ex, ey, ez = end_effector.get_global_position()
                    dist = math.sqrt((target_x - ex)**2 + (target_y - ey)**2 + (target_z - ez)**2)

                # move the base if required, kind of like how the rover will drive around
                if move_base and joint.parent is None:
                    # find the heading to the target
                    target_angle = math.atan2(target_y - joint.offset_y, target_x - joint.offset_x)
                    current_angle = joint.angle_z
                    
                    # Calculate the shortest turn needed to face the target
                    angle_diff = (target_angle - current_angle + math.pi) % (2 * math.pi) - math.pi
                    
                    # Clamp the turn to our speed limit and apply it
                    turn_step = max(-max_angle_step, min(max_angle_step, math.degrees(angle_diff)))
                    joint.set_angles(az = math.degrees(current_angle) + turn_step)
                    total_angular += math.radians(turn_step)
                    
                    # figure out the forward vector
                    forward_x = math.cos(joint.angle_z)
                    forward_y = math.sin(joint.angle_z)
                    
                    # save our original position
                    orig_x = joint.offset_x
                    orig_y = joint.offset_y
                    
                    # wiggle the rover forward along its vector
                    joint.set_position(x=orig_x + (wiggle * forward_x), 
                                       y=orig_y + (wiggle * forward_y))
                    
                    # measure the temporary end effector distance from the test location
                    tex, tey, tez = end_effector.get_global_position()
                    new_dist = math.sqrt((target_x - tex)**2 + (target_y - tey)**2 + (target_z - tez)**2)
                    
                    # Calculate gradient and apply the step
                    gradient = (new_dist - dist) / wiggle
                    step = -(learning_rate * 0.5) * gradient 
                    step = max(-max_pos_step, min(max_pos_step, step))
                    
                    joint.set_position(x=orig_x + (step * forward_x), 
                                       y=orig_y + (step * forward_y))
                    total_linear += step
                    
                    # update positions for the next iteration
                    ex, ey, ez = end_effector.get_global_position()
                    dist = math.sqrt((target_x - ex)**2 + (target_y - ey)**2 + (target_z - ez)**2)
        
        return total_linear, total_angular

    def print_chain_positions(self, depth=0):
        x, y, z = self.get_global_position()
        indent = "  " * depth
        print(f"{indent}- {self.name}: Pos (X:{x:.1f}, Y:{y:.1f}, Z:{z:.1f})")
        for child in self.children:
            child.print_chain_positions(depth + 1)

    # --- 3D VISUALIZATION --- (ai generated)
    def draw(self, ax):
        x, y, z = self.get_global_position()
        
        if self.parent is None:
            ax.plot([x], [y], [z], 'ko', markersize=8, label="Base Origin")
        else:
            ax.plot([x], [y], [z], 'ro', markersize=6)
            
        for child in self.children:
            cx, cy, cz = child.get_global_position()
            ax.plot([x, cx], [y, cy], [z, cz], 'b-', linewidth=3)
            child.draw(ax)

    def plot_chain(self, title="3D Kinematic Chain"):
        fig = plt.figure(figsize=(8, 8))
        ax = fig.add_subplot(111, projection='3d') 
        
        self.draw(ax)
        
        ax.set_title(title)
        ax.set_xlabel("X Axis")
        ax.set_ylabel("Y Axis")
        ax.set_zlabel("Z Axis")
        
        # Keep the axes square so the arm doesn't warp
        ax.set_xlim(-15, 15)
        ax.set_ylim(-15, 15)
        ax.set_zlim(0, 20)
        
        plt.show()
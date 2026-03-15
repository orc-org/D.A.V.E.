# arm_interfaces

This package contains the ROS2 interface definitions for the arm system, outside of the standard interfaces (Point, Twist JointState e.t.c). 

By keeping these in a separate package, any other module (Python or C++) can interact with the arm by agreed-upon standards.

## Contents

### Messages (`msg/`)
*   **ArmStatus.msg**: Contains `current_dist` (float64) representing the distance from the tool tip to the target.

### Services (`srv/`)
*   **CalibrateArm.srv**: A request/response used to reset all arm joints to zero.

### Actions (`action/`)
*   **MoveToPoint.action**: An action to move the arm tip to a specific coordinate. Provides feedback on progress percentage.

## Usage in other packages
Add this to your `package.xml`:
```xml
<depend>arm_interfaces</depend>
```
And import in Python:
```python
from arm_interfaces.msg import ArmStatus
```

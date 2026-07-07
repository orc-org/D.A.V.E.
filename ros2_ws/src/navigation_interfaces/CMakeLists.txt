cmake_minimum_required(VERSION 3.8)
project(navigation_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/BearingString.msg"
  "msg/PositionString.msg"
  
  "srv/GetNav.srv"
  "srv/SetNav.srv"
  
  "srv/GetHome.srv"
  "srv/SetHome.srv"
  
  "srv/GetTarget.srv"
  "srv/SetTarget.srv"

  "srv/GoHome.srv"
  "srv/SerialSend.srv"

  "srv/ToggleDisplayTopicInfo.srv"
  "srv/TogglePreference.srv"

  "action/NavigationAction.action"

  "msg/Velocity.msg"

  "srv/SelectRoute.srv"
  "srv/VoidService.srv"

  "srv/ResetHome.srv"

  "srv/AddLocalWaypoint.srv"
  "srv/AddLocalWaypointAtIndex.srv"
  "srv/AddLocalObstacle.srv"

  "srv/AddGeodeticWaypoint.srv"
  "srv/AddGeodeticWaypointAtIndex.srv"
  "srv/AddGeodeticObstacle.srv"

  "srv/AddEarthCentredWaypoint.srv"
  "srv/AddEarthCentredWaypointAtIndex.srv"
  "srv/AddEarthCentredObstacle.srv"

)

ament_export_dependencies(rosidl_default_runtime)

ament_package()

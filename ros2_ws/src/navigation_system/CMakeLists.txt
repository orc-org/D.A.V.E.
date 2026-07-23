cmake_minimum_required(VERSION 3.8)
project(navigation_system)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Resolve the workspace root so CMake can find built/installed packages
get_filename_component(ROS2_WS "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
list(APPEND CMAKE_PREFIX_PATH "${ROS2_WS}/install")

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(navigation_interfaces REQUIRED)
find_package(serial_interfaces REQUIRED)
find_package(serial_communication REQUIRED)

install(
  DIRECTORY ${ROS2_WS}/src/serial_communication/include/serial_communication
  DESTINATION include
)

set(SOURCE_FILES
  src/Waypoint.cpp 
  src/Obstacle.cpp 
  src/ListOfWaypoints.cpp
  src/NAVnode.cpp
  ${ROS2_WS}/src/serial_communication/src/SerialPort.cpp
)

add_executable(NAVnode ${SOURCE_FILES} )
ament_target_dependencies(NAVnode rclcpp navigation_interfaces serial_interfaces serial_communication)

target_include_directories(NAVnode PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/navigation_system>
  $<INSTALL_INTERFACE:include/navigation_system>
)

install(TARGETS
  NAVnode
  DESTINATION lib/${PROJECT_NAME})


ament_package()

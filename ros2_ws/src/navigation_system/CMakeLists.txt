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
find_package(rclcpp_action)
find_package(rclcpp_components)
find_package(navigation_interfaces REQUIRED)
find_package(serial_interfaces REQUIRED)
find_package(serial_communication REQUIRED)
find_package(raylib REQUIRED)

install(
  DIRECTORY ${ROS2_WS}/src/serial_communication/include/serial_communication
  DESTINATION include
)

install(
  DIRECTORY include/
  DESTINATION include
)

set(SHARED_SOURCES
  src/Waypoint.cpp
  src/Obstacle.cpp
  src/ListOfWaypoints.cpp
  src/Sprite.cpp
  ${ROS2_WS}/src/serial_communication/src/SerialPort.cpp
)

set(NAVNODE_SOURCES
  ${SHARED_SOURCES}
  src/NAVnode.cpp
)

set(NAVDISPLAY_SOURCES
  ${SHARED_SOURCES}
  src/NAVDisplay.cpp
)

add_executable(NAVnode ${NAVNODE_SOURCES})
ament_target_dependencies(NAVnode rclcpp rclcpp_action rclcpp_components navigation_interfaces serial_interfaces serial_communication)
target_link_libraries(NAVnode raylib)

add_executable(NAVDisplay ${NAVDISPLAY_SOURCES})
target_link_libraries(NAVDisplay raylib)
ament_target_dependencies(NAVDisplay rclcpp rclcpp_action navigation_interfaces raylib serial_communication)

target_include_directories(NAVnode PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/navigation_system>
  $<INSTALL_INTERFACE:include/navigation_system>
  $<BUILD_INTERFACE:${ROS2_WS}/src/serial_communication/include/serial_communication>
  $<INSTALL_INTERFACE:include/serial_communication>
  $<BUILD_INTERFACE:${ROS2_WS}/install/serial_interfaces/include/serial_interfaces>
  $<INSTALL_INTERFACE:include/serial_interfaces/serial_interfaces>
)

target_include_directories(NAVDisplay PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/navigation_system>
  $<INSTALL_INTERFACE:include/navigation_system>
  $<BUILD_INTERFACE:${ROS2_WS}/src/serial_communication/include/serial_communication>
  $<INSTALL_INTERFACE:include/serial_communication>
  $<BUILD_INTERFACE:${ROS2_WS}/install/serial_interfaces/include/serial_interfaces>
  $<INSTALL_INTERFACE:include/serial_interfaces/serial_interfaces>
)

ament_export_include_directories(include)

# install targets
install(TARGETS
  NAVnode
  NAVDisplay
  DESTINATION lib/${PROJECT_NAME})

install(PROGRAMS
  src/gps_bridge_node.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()

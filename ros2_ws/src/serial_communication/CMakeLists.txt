cmake_minimum_required(VERSION 3.8)
project(serial_communication)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(serial_interfaces REQUIRED)

set(SOURCE_FILES
  src/SerialPort.cpp 
)


add_executable(XLRnode 
  src/XLRnode.cpp
  src/SerialPort.cpp
)
ament_target_dependencies(XLRnode rclcpp serial_interfaces)

target_include_directories(XLRnode PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/serial_communication>
  $<INSTALL_INTERFACE:include/serial_communication>
)

ament_export_include_directories(include/serial_communication)

install(
  DIRECTORY include/serial_communication
  DESTINATION include
)

install(TARGETS
  XLRnode
  DESTINATION lib/${PROJECT_NAME})



ament_package()

cmake_minimum_required(VERSION 3.8)
project(serial_communication)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(serial_interfaces REQUIRED)


add_executable(XLRnode src/XLRnode.cpp)
ament_target_dependencies(XLRnode rclcpp serial_interfaces)

install(TARGETS
  XLRnode
  DESTINATION lib/${PROJECT_NAME})



ament_package()

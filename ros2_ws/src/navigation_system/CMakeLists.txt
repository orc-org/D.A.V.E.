cmake_minimum_required(VERSION 3.8)
project(navigation_system)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(navigation_interfaces REQUIRED)


add_executable(GPSnode src/GPSnode.cpp)
ament_target_dependencies(GPSnode rclcpp navigation_interfaces)

install(TARGETS
  GPSnode
  DESTINATION lib/${PROJECT_NAME})


ament_package()

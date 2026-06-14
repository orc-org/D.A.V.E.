cmake_minimum_required(VERSION 3.8)
project(camera_handler)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(OpenCV REQUIRED)

# Add Executable
add_executable(multi_camera_node src/multi_camera_node.cpp)

# Target dependencies
target_link_libraries(multi_camera_node ${OpenCV_LIBRARIES})
ament_target_dependencies(multi_camera_node
  rclcpp
  sensor_msgs
  cv_bridge
  image_transport
)

# Install rules
install(TARGETS
  multi_camera_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()

cmake_minimum_required(VERSION 3.8)
project(drive_package)

# Compile options
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)

# -----------------------------
# C++ joystick drive node
# -----------------------------
add_executable(drive_node src/drive_node.cpp)

ament_target_dependencies(drive_node
  rclcpp
  sensor_msgs
  std_msgs
)

install(TARGETS
  drive_node
  DESTINATION lib/${PROJECT_NAME}
)

# Python visualizer thingy
install(PROGRAMS
  scripts/input_viewer.py
  DESTINATION lib/${PROJECT_NAME}
)

# Install the ps5 png to use for background
install(DIRECTORY resource/
  DESTINATION share/${PROJECT_NAME}/resource
)

# -----------------------------
# Linting
# -----------------------------
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()



ament_package()

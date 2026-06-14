cmake_minimum_required(VERSION 3.8)
project(enigma_machine)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(enigma_machine_interfaces REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(arm_interfaces REQUIRED)

# c++ stuff
# Include cpp "include" directory (because the tutorial guy defined header files there)
include_directories(include)

#create c++ executables
add_executable(enigma_node src/EnigmaMachine.cpp)
ament_target_dependencies(enigma_node rclcpp rclcpp_action enigma_machine_interfaces arm_interfaces)

#install cpp excecutables
install(TARGETS
  enigma_node
  DESTINATION lib/${PROJECT_NAME}
)

#python stuff
#install python modules
ament_python_install_package(${PROJECT_NAME})

#python nodes should be made executable when you declare them (using the "chmod +x py_nofe.py" command in command line)
#therefore we do not need a section to declare our executables like we did for c++ nodes

#install python executables
install(PROGRAMS
  scripts/py_node.py
  scripts/morse_recorder.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()

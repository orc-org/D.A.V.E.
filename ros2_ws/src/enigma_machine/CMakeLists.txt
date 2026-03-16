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

# c++ stuff
# Include cpp "include" directory (because the tutorial guy defined header files there)
include_directories(include)

#create c++ executables
#add_executable(nodeName path/to/node)
#ametn_target_dependencies(nodeName dependencies)

#install cpp excecutables
#install(TARGETS
  #cppExcecutable1
  #DESTINATION lib/${PROJECT_NAME}
#)

#python stuff
#install python modules
ament_python_install_package(${PROJECT_NAME})

#python nodes should be made executable when you declare them (using the "chmod +x py_nofe.py" command in command line)
#therefore we do not need a section to declare our executables like we did for c++ nodes

#install python executables
install(PROGRAMS
  scripts/py_node.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()

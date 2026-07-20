cmake_minimum_required(VERSION 3.8)
project(navigation_system)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(navigation_interfaces REQUIRED)
find_package(navigation_system REQUIRED)

set(SOURCE_FILES
  src/Waypoint.cpp 
  src/Obstacle.cpp 
  src/ListOfWaypoints.cpp
  src/NAVnode.cpp
)

add_executable(NAVnode ${SOURCE_FILES} )
ament_target_dependencies(NAVnode rclcpp navigation_interfaces)

target_include_directories(NAVnode PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/navigation_system>
  $<INSTALL_INTERFACE:include/navigation_system>
)

install(TARGETS
  NAVnode
  DESTINATION lib/${PROJECT_NAME})


ament_package()

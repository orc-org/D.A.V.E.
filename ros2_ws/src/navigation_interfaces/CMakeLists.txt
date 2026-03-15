cmake_minimum_required(VERSION 3.8)
project(navigation_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Bearing.msg"
  "msg/Position.msg"
  "srv/GetNav.srv"
  "srv/SetNav.srv"

  #declare any other interfaces that the above interfaces depend on below
  #DEPENDENCIES geometry_msgs #an interface used for some math stuff (for example)
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()

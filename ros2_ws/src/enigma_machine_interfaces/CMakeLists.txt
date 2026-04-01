cmake_minimum_required(VERSION 3.8)
project(enigma_machine_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Morse.msg"
  "msg/MorseSignal.msg"
  "srv/GetPassword.srv"
  "srv/SetMessage.srv"
  "action/MorseSend.action"
  "action/Encode.action"
  "action/Decode.action"
  "action/GetMorseSignalToDecoder.action"
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()

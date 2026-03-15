// generated from rosidl_generator_cpp/resource/rosidl_generator_cpp__visibility_control.hpp.in
// generated code does not contain a copyright notice

#ifndef NAVSYSTEM__MSG__ROSIDL_GENERATOR_CPP__VISIBILITY_CONTROL_HPP_
#define NAVSYSTEM__MSG__ROSIDL_GENERATOR_CPP__VISIBILITY_CONTROL_HPP_

#ifdef __cplusplus
extern "C"
{
#endif

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define ROSIDL_GENERATOR_CPP_EXPORT_navsystem __attribute__ ((dllexport))
    #define ROSIDL_GENERATOR_CPP_IMPORT_navsystem __attribute__ ((dllimport))
  #else
    #define ROSIDL_GENERATOR_CPP_EXPORT_navsystem __declspec(dllexport)
    #define ROSIDL_GENERATOR_CPP_IMPORT_navsystem __declspec(dllimport)
  #endif
  #ifdef ROSIDL_GENERATOR_CPP_BUILDING_DLL_navsystem
    #define ROSIDL_GENERATOR_CPP_PUBLIC_navsystem ROSIDL_GENERATOR_CPP_EXPORT_navsystem
  #else
    #define ROSIDL_GENERATOR_CPP_PUBLIC_navsystem ROSIDL_GENERATOR_CPP_IMPORT_navsystem
  #endif
#else
  #define ROSIDL_GENERATOR_CPP_EXPORT_navsystem __attribute__ ((visibility("default")))
  #define ROSIDL_GENERATOR_CPP_IMPORT_navsystem
  #if __GNUC__ >= 4
    #define ROSIDL_GENERATOR_CPP_PUBLIC_navsystem __attribute__ ((visibility("default")))
  #else
    #define ROSIDL_GENERATOR_CPP_PUBLIC_navsystem
  #endif
#endif

#ifdef __cplusplus
}
#endif

#endif  // NAVSYSTEM__MSG__ROSIDL_GENERATOR_CPP__VISIBILITY_CONTROL_HPP_

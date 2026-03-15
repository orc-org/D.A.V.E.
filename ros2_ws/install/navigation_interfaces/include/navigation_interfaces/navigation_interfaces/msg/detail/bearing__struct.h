// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from navigation_interfaces:msg/Bearing.idl
// generated code does not contain a copyright notice

#ifndef NAVIGATION_INTERFACES__MSG__DETAIL__BEARING__STRUCT_H_
#define NAVIGATION_INTERFACES__MSG__DETAIL__BEARING__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'bearing'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/Bearing in the package navigation_interfaces.
typedef struct navigation_interfaces__msg__Bearing
{
  rosidl_runtime_c__String bearing;
} navigation_interfaces__msg__Bearing;

// Struct for a sequence of navigation_interfaces__msg__Bearing.
typedef struct navigation_interfaces__msg__Bearing__Sequence
{
  navigation_interfaces__msg__Bearing * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} navigation_interfaces__msg__Bearing__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // NAVIGATION_INTERFACES__MSG__DETAIL__BEARING__STRUCT_H_

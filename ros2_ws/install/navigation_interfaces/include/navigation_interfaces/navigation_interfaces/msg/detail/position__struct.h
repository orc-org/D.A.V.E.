// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from navigation_interfaces:msg/Position.idl
// generated code does not contain a copyright notice

#ifndef NAVIGATION_INTERFACES__MSG__DETAIL__POSITION__STRUCT_H_
#define NAVIGATION_INTERFACES__MSG__DETAIL__POSITION__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'position'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/Position in the package navigation_interfaces.
typedef struct navigation_interfaces__msg__Position
{
  rosidl_runtime_c__String position;
} navigation_interfaces__msg__Position;

// Struct for a sequence of navigation_interfaces__msg__Position.
typedef struct navigation_interfaces__msg__Position__Sequence
{
  navigation_interfaces__msg__Position * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} navigation_interfaces__msg__Position__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // NAVIGATION_INTERFACES__MSG__DETAIL__POSITION__STRUCT_H_

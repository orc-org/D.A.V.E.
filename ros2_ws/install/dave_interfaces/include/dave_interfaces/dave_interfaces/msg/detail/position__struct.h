// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from dave_interfaces:msg/Position.idl
// generated code does not contain a copyright notice

#ifndef DAVE_INTERFACES__MSG__DETAIL__POSITION__STRUCT_H_
#define DAVE_INTERFACES__MSG__DETAIL__POSITION__STRUCT_H_

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

/// Struct defined in msg/Position in the package dave_interfaces.
typedef struct dave_interfaces__msg__Position
{
  rosidl_runtime_c__String position;
} dave_interfaces__msg__Position;

// Struct for a sequence of dave_interfaces__msg__Position.
typedef struct dave_interfaces__msg__Position__Sequence
{
  dave_interfaces__msg__Position * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} dave_interfaces__msg__Position__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // DAVE_INTERFACES__MSG__DETAIL__POSITION__STRUCT_H_

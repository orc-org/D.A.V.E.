// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from navsystem:msg/Message.idl
// generated code does not contain a copyright notice

#ifndef NAVSYSTEM__MSG__DETAIL__MESSAGE__STRUCT_H_
#define NAVSYSTEM__MSG__DETAIL__MESSAGE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'message'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/Message in the package navsystem.
typedef struct navsystem__msg__Message
{
  rosidl_runtime_c__String message;
} navsystem__msg__Message;

// Struct for a sequence of navsystem__msg__Message.
typedef struct navsystem__msg__Message__Sequence
{
  navsystem__msg__Message * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} navsystem__msg__Message__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // NAVSYSTEM__MSG__DETAIL__MESSAGE__STRUCT_H_

// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from navsystem:srv/GetNav.idl
// generated code does not contain a copyright notice

#ifndef NAVSYSTEM__SRV__DETAIL__GET_NAV__STRUCT_H_
#define NAVSYSTEM__SRV__DETAIL__GET_NAV__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'function'
#include "rosidl_runtime_c/string.h"

/// Struct defined in srv/GetNav in the package navsystem.
typedef struct navsystem__srv__GetNav_Request
{
  rosidl_runtime_c__String function;
} navsystem__srv__GetNav_Request;

// Struct for a sequence of navsystem__srv__GetNav_Request.
typedef struct navsystem__srv__GetNav_Request__Sequence
{
  navsystem__srv__GetNav_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} navsystem__srv__GetNav_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'returned'
// already included above
// #include "rosidl_runtime_c/string.h"

/// Struct defined in srv/GetNav in the package navsystem.
typedef struct navsystem__srv__GetNav_Response
{
  rosidl_runtime_c__String returned;
} navsystem__srv__GetNav_Response;

// Struct for a sequence of navsystem__srv__GetNav_Response.
typedef struct navsystem__srv__GetNav_Response__Sequence
{
  navsystem__srv__GetNav_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} navsystem__srv__GetNav_Response__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // NAVSYSTEM__SRV__DETAIL__GET_NAV__STRUCT_H_

// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from custom_interfaces:srv/SetNav.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACES__SRV__DETAIL__SET_NAV__STRUCT_H_
#define CUSTOM_INTERFACES__SRV__DETAIL__SET_NAV__STRUCT_H_

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

/// Struct defined in srv/SetNav in the package custom_interfaces.
typedef struct custom_interfaces__srv__SetNav_Request
{
  rosidl_runtime_c__String function;
} custom_interfaces__srv__SetNav_Request;

// Struct for a sequence of custom_interfaces__srv__SetNav_Request.
typedef struct custom_interfaces__srv__SetNav_Request__Sequence
{
  custom_interfaces__srv__SetNav_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interfaces__srv__SetNav_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'returned'
// already included above
// #include "rosidl_runtime_c/string.h"

/// Struct defined in srv/SetNav in the package custom_interfaces.
typedef struct custom_interfaces__srv__SetNav_Response
{
  rosidl_runtime_c__String returned;
} custom_interfaces__srv__SetNav_Response;

// Struct for a sequence of custom_interfaces__srv__SetNav_Response.
typedef struct custom_interfaces__srv__SetNav_Response__Sequence
{
  custom_interfaces__srv__SetNav_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interfaces__srv__SetNav_Response__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CUSTOM_INTERFACES__SRV__DETAIL__SET_NAV__STRUCT_H_

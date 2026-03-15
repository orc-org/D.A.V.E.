// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from navigation_interfaces:srv/GetSetNav.idl
// generated code does not contain a copyright notice

#ifndef NAVIGATION_INTERFACES__SRV__DETAIL__GET_SET_NAV__STRUCT_H_
#define NAVIGATION_INTERFACES__SRV__DETAIL__GET_SET_NAV__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'function2call'
#include "rosidl_runtime_c/string.h"

/// Struct defined in srv/GetSetNav in the package navigation_interfaces.
typedef struct navigation_interfaces__srv__GetSetNav_Request
{
  rosidl_runtime_c__String function2call;
} navigation_interfaces__srv__GetSetNav_Request;

// Struct for a sequence of navigation_interfaces__srv__GetSetNav_Request.
typedef struct navigation_interfaces__srv__GetSetNav_Request__Sequence
{
  navigation_interfaces__srv__GetSetNav_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} navigation_interfaces__srv__GetSetNav_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'returnedmessage'
// already included above
// #include "rosidl_runtime_c/string.h"

/// Struct defined in srv/GetSetNav in the package navigation_interfaces.
typedef struct navigation_interfaces__srv__GetSetNav_Response
{
  rosidl_runtime_c__String returnedmessage;
} navigation_interfaces__srv__GetSetNav_Response;

// Struct for a sequence of navigation_interfaces__srv__GetSetNav_Response.
typedef struct navigation_interfaces__srv__GetSetNav_Response__Sequence
{
  navigation_interfaces__srv__GetSetNav_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} navigation_interfaces__srv__GetSetNav_Response__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // NAVIGATION_INTERFACES__SRV__DETAIL__GET_SET_NAV__STRUCT_H_

// generated from rosidl_typesupport_fastrtps_c/resource/idl__type_support_c.cpp.em
// with input from navigation_interfaces:srv/GetNav.idl
// generated code does not contain a copyright notice
#include "navigation_interfaces/srv/detail/get_nav__rosidl_typesupport_fastrtps_c.h"


#include <cassert>
#include <limits>
#include <string>
#include "rosidl_typesupport_fastrtps_c/identifier.h"
#include "rosidl_typesupport_fastrtps_c/wstring_conversion.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "navigation_interfaces/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "navigation_interfaces/srv/detail/get_nav__struct.h"
#include "navigation_interfaces/srv/detail/get_nav__functions.h"
#include "fastcdr/Cdr.h"

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

// includes and forward declarations of message dependencies and their conversion functions

#if defined(__cplusplus)
extern "C"
{
#endif

#include "rosidl_runtime_c/string.h"  // function2call
#include "rosidl_runtime_c/string_functions.h"  // function2call

// forward declare type support functions


using _GetNav_Request__ros_msg_type = navigation_interfaces__srv__GetNav_Request;

static bool _GetNav_Request__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  const _GetNav_Request__ros_msg_type * ros_message = static_cast<const _GetNav_Request__ros_msg_type *>(untyped_ros_message);
  // Field name: function2call
  {
    const rosidl_runtime_c__String * str = &ros_message->function2call;
    if (str->capacity == 0 || str->capacity <= str->size) {
      fprintf(stderr, "string capacity not greater than size\n");
      return false;
    }
    if (str->data[str->size] != '\0') {
      fprintf(stderr, "string not null-terminated\n");
      return false;
    }
    cdr << str->data;
  }

  return true;
}

static bool _GetNav_Request__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  _GetNav_Request__ros_msg_type * ros_message = static_cast<_GetNav_Request__ros_msg_type *>(untyped_ros_message);
  // Field name: function2call
  {
    std::string tmp;
    cdr >> tmp;
    if (!ros_message->function2call.data) {
      rosidl_runtime_c__String__init(&ros_message->function2call);
    }
    bool succeeded = rosidl_runtime_c__String__assign(
      &ros_message->function2call,
      tmp.c_str());
    if (!succeeded) {
      fprintf(stderr, "failed to assign string into field 'function2call'\n");
      return false;
    }
  }

  return true;
}  // NOLINT(readability/fn_size)

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_navigation_interfaces
size_t get_serialized_size_navigation_interfaces__srv__GetNav_Request(
  const void * untyped_ros_message,
  size_t current_alignment)
{
  const _GetNav_Request__ros_msg_type * ros_message = static_cast<const _GetNav_Request__ros_msg_type *>(untyped_ros_message);
  (void)ros_message;
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // field.name function2call
  current_alignment += padding +
    eprosima::fastcdr::Cdr::alignment(current_alignment, padding) +
    (ros_message->function2call.size + 1);

  return current_alignment - initial_alignment;
}

static uint32_t _GetNav_Request__get_serialized_size(const void * untyped_ros_message)
{
  return static_cast<uint32_t>(
    get_serialized_size_navigation_interfaces__srv__GetNav_Request(
      untyped_ros_message, 0));
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_navigation_interfaces
size_t max_serialized_size_navigation_interfaces__srv__GetNav_Request(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  size_t last_member_size = 0;
  (void)last_member_size;
  (void)padding;
  (void)wchar_size;

  full_bounded = true;
  is_plain = true;

  // member: function2call
  {
    size_t array_size = 1;

    full_bounded = false;
    is_plain = false;
    for (size_t index = 0; index < array_size; ++index) {
      current_alignment += padding +
        eprosima::fastcdr::Cdr::alignment(current_alignment, padding) +
        1;
    }
  }

  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = navigation_interfaces__srv__GetNav_Request;
    is_plain =
      (
      offsetof(DataType, function2call) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static size_t _GetNav_Request__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_navigation_interfaces__srv__GetNav_Request(
    full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}


static message_type_support_callbacks_t __callbacks_GetNav_Request = {
  "navigation_interfaces::srv",
  "GetNav_Request",
  _GetNav_Request__cdr_serialize,
  _GetNav_Request__cdr_deserialize,
  _GetNav_Request__get_serialized_size,
  _GetNav_Request__max_serialized_size
};

static rosidl_message_type_support_t _GetNav_Request__type_support = {
  rosidl_typesupport_fastrtps_c__identifier,
  &__callbacks_GetNav_Request,
  get_message_typesupport_handle_function,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, navigation_interfaces, srv, GetNav_Request)() {
  return &_GetNav_Request__type_support;
}

#if defined(__cplusplus)
}
#endif

// already included above
// #include <cassert>
// already included above
// #include <limits>
// already included above
// #include <string>
// already included above
// #include "rosidl_typesupport_fastrtps_c/identifier.h"
// already included above
// #include "rosidl_typesupport_fastrtps_c/wstring_conversion.hpp"
// already included above
// #include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
// already included above
// #include "navigation_interfaces/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
// already included above
// #include "navigation_interfaces/srv/detail/get_nav__struct.h"
// already included above
// #include "navigation_interfaces/srv/detail/get_nav__functions.h"
// already included above
// #include "fastcdr/Cdr.h"

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

// includes and forward declarations of message dependencies and their conversion functions

#if defined(__cplusplus)
extern "C"
{
#endif

// already included above
// #include "rosidl_runtime_c/string.h"  // returnedmessage
// already included above
// #include "rosidl_runtime_c/string_functions.h"  // returnedmessage

// forward declare type support functions


using _GetNav_Response__ros_msg_type = navigation_interfaces__srv__GetNav_Response;

static bool _GetNav_Response__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  const _GetNav_Response__ros_msg_type * ros_message = static_cast<const _GetNav_Response__ros_msg_type *>(untyped_ros_message);
  // Field name: returnedmessage
  {
    const rosidl_runtime_c__String * str = &ros_message->returnedmessage;
    if (str->capacity == 0 || str->capacity <= str->size) {
      fprintf(stderr, "string capacity not greater than size\n");
      return false;
    }
    if (str->data[str->size] != '\0') {
      fprintf(stderr, "string not null-terminated\n");
      return false;
    }
    cdr << str->data;
  }

  return true;
}

static bool _GetNav_Response__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  _GetNav_Response__ros_msg_type * ros_message = static_cast<_GetNav_Response__ros_msg_type *>(untyped_ros_message);
  // Field name: returnedmessage
  {
    std::string tmp;
    cdr >> tmp;
    if (!ros_message->returnedmessage.data) {
      rosidl_runtime_c__String__init(&ros_message->returnedmessage);
    }
    bool succeeded = rosidl_runtime_c__String__assign(
      &ros_message->returnedmessage,
      tmp.c_str());
    if (!succeeded) {
      fprintf(stderr, "failed to assign string into field 'returnedmessage'\n");
      return false;
    }
  }

  return true;
}  // NOLINT(readability/fn_size)

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_navigation_interfaces
size_t get_serialized_size_navigation_interfaces__srv__GetNav_Response(
  const void * untyped_ros_message,
  size_t current_alignment)
{
  const _GetNav_Response__ros_msg_type * ros_message = static_cast<const _GetNav_Response__ros_msg_type *>(untyped_ros_message);
  (void)ros_message;
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // field.name returnedmessage
  current_alignment += padding +
    eprosima::fastcdr::Cdr::alignment(current_alignment, padding) +
    (ros_message->returnedmessage.size + 1);

  return current_alignment - initial_alignment;
}

static uint32_t _GetNav_Response__get_serialized_size(const void * untyped_ros_message)
{
  return static_cast<uint32_t>(
    get_serialized_size_navigation_interfaces__srv__GetNav_Response(
      untyped_ros_message, 0));
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_navigation_interfaces
size_t max_serialized_size_navigation_interfaces__srv__GetNav_Response(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  size_t last_member_size = 0;
  (void)last_member_size;
  (void)padding;
  (void)wchar_size;

  full_bounded = true;
  is_plain = true;

  // member: returnedmessage
  {
    size_t array_size = 1;

    full_bounded = false;
    is_plain = false;
    for (size_t index = 0; index < array_size; ++index) {
      current_alignment += padding +
        eprosima::fastcdr::Cdr::alignment(current_alignment, padding) +
        1;
    }
  }

  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = navigation_interfaces__srv__GetNav_Response;
    is_plain =
      (
      offsetof(DataType, returnedmessage) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static size_t _GetNav_Response__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_navigation_interfaces__srv__GetNav_Response(
    full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}


static message_type_support_callbacks_t __callbacks_GetNav_Response = {
  "navigation_interfaces::srv",
  "GetNav_Response",
  _GetNav_Response__cdr_serialize,
  _GetNav_Response__cdr_deserialize,
  _GetNav_Response__get_serialized_size,
  _GetNav_Response__max_serialized_size
};

static rosidl_message_type_support_t _GetNav_Response__type_support = {
  rosidl_typesupport_fastrtps_c__identifier,
  &__callbacks_GetNav_Response,
  get_message_typesupport_handle_function,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, navigation_interfaces, srv, GetNav_Response)() {
  return &_GetNav_Response__type_support;
}

#if defined(__cplusplus)
}
#endif

#include "rosidl_typesupport_fastrtps_cpp/service_type_support.h"
#include "rosidl_typesupport_cpp/service_type_support.hpp"
// already included above
// #include "rosidl_typesupport_fastrtps_c/identifier.h"
// already included above
// #include "navigation_interfaces/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "navigation_interfaces/srv/get_nav.h"

#if defined(__cplusplus)
extern "C"
{
#endif

static service_type_support_callbacks_t GetNav__callbacks = {
  "navigation_interfaces::srv",
  "GetNav",
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, navigation_interfaces, srv, GetNav_Request)(),
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, navigation_interfaces, srv, GetNav_Response)(),
};

static rosidl_service_type_support_t GetNav__handle = {
  rosidl_typesupport_fastrtps_c__identifier,
  &GetNav__callbacks,
  get_service_typesupport_handle_function,
};

const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, navigation_interfaces, srv, GetNav)() {
  return &GetNav__handle;
}

#if defined(__cplusplus)
}
#endif

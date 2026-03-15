// generated from rosidl_typesupport_fastrtps_cpp/resource/idl__type_support.cpp.em
// with input from navigation_interfaces:msg/Bearing.idl
// generated code does not contain a copyright notice
#include "navigation_interfaces/msg/detail/bearing__rosidl_typesupport_fastrtps_cpp.hpp"
#include "navigation_interfaces/msg/detail/bearing__struct.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_fastrtps_cpp/identifier.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_fastrtps_cpp/wstring_conversion.hpp"
#include "fastcdr/Cdr.h"


// forward declaration of message dependencies and their conversion functions

namespace navigation_interfaces
{

namespace msg
{

namespace typesupport_fastrtps_cpp
{

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_navigation_interfaces
cdr_serialize(
  const navigation_interfaces::msg::Bearing & ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  // Member: bearing
  cdr << ros_message.bearing;
  return true;
}

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_navigation_interfaces
cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  navigation_interfaces::msg::Bearing & ros_message)
{
  // Member: bearing
  cdr >> ros_message.bearing;

  return true;
}  // NOLINT(readability/fn_size)

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_navigation_interfaces
get_serialized_size(
  const navigation_interfaces::msg::Bearing & ros_message,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // Member: bearing
  current_alignment += padding +
    eprosima::fastcdr::Cdr::alignment(current_alignment, padding) +
    (ros_message.bearing.size() + 1);

  return current_alignment - initial_alignment;
}

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_navigation_interfaces
max_serialized_size_Bearing(
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


  // Member: bearing
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
    using DataType = navigation_interfaces::msg::Bearing;
    is_plain =
      (
      offsetof(DataType, bearing) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static bool _Bearing__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  auto typed_message =
    static_cast<const navigation_interfaces::msg::Bearing *>(
    untyped_ros_message);
  return cdr_serialize(*typed_message, cdr);
}

static bool _Bearing__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  auto typed_message =
    static_cast<navigation_interfaces::msg::Bearing *>(
    untyped_ros_message);
  return cdr_deserialize(cdr, *typed_message);
}

static uint32_t _Bearing__get_serialized_size(
  const void * untyped_ros_message)
{
  auto typed_message =
    static_cast<const navigation_interfaces::msg::Bearing *>(
    untyped_ros_message);
  return static_cast<uint32_t>(get_serialized_size(*typed_message, 0));
}

static size_t _Bearing__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_Bearing(full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}

static message_type_support_callbacks_t _Bearing__callbacks = {
  "navigation_interfaces::msg",
  "Bearing",
  _Bearing__cdr_serialize,
  _Bearing__cdr_deserialize,
  _Bearing__get_serialized_size,
  _Bearing__max_serialized_size
};

static rosidl_message_type_support_t _Bearing__handle = {
  rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
  &_Bearing__callbacks,
  get_message_typesupport_handle_function,
};

}  // namespace typesupport_fastrtps_cpp

}  // namespace msg

}  // namespace navigation_interfaces

namespace rosidl_typesupport_fastrtps_cpp
{

template<>
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_EXPORT_navigation_interfaces
const rosidl_message_type_support_t *
get_message_type_support_handle<navigation_interfaces::msg::Bearing>()
{
  return &navigation_interfaces::msg::typesupport_fastrtps_cpp::_Bearing__handle;
}

}  // namespace rosidl_typesupport_fastrtps_cpp

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, navigation_interfaces, msg, Bearing)() {
  return &navigation_interfaces::msg::typesupport_fastrtps_cpp::_Bearing__handle;
}

#ifdef __cplusplus
}
#endif

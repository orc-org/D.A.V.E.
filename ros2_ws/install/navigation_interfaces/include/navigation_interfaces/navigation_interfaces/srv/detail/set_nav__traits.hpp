// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from navigation_interfaces:srv/SetNav.idl
// generated code does not contain a copyright notice

#ifndef NAVIGATION_INTERFACES__SRV__DETAIL__SET_NAV__TRAITS_HPP_
#define NAVIGATION_INTERFACES__SRV__DETAIL__SET_NAV__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "navigation_interfaces/srv/detail/set_nav__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace navigation_interfaces
{

namespace srv
{

inline void to_flow_style_yaml(
  const SetNav_Request & msg,
  std::ostream & out)
{
  out << "{";
  // member: function2call
  {
    out << "function2call: ";
    rosidl_generator_traits::value_to_yaml(msg.function2call, out);
    out << ", ";
  }

  // member: arguments
  {
    out << "arguments: ";
    rosidl_generator_traits::value_to_yaml(msg.arguments, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const SetNav_Request & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: function2call
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "function2call: ";
    rosidl_generator_traits::value_to_yaml(msg.function2call, out);
    out << "\n";
  }

  // member: arguments
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "arguments: ";
    rosidl_generator_traits::value_to_yaml(msg.arguments, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const SetNav_Request & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace srv

}  // namespace navigation_interfaces

namespace rosidl_generator_traits
{

[[deprecated("use navigation_interfaces::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const navigation_interfaces::srv::SetNav_Request & msg,
  std::ostream & out, size_t indentation = 0)
{
  navigation_interfaces::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use navigation_interfaces::srv::to_yaml() instead")]]
inline std::string to_yaml(const navigation_interfaces::srv::SetNav_Request & msg)
{
  return navigation_interfaces::srv::to_yaml(msg);
}

template<>
inline const char * data_type<navigation_interfaces::srv::SetNav_Request>()
{
  return "navigation_interfaces::srv::SetNav_Request";
}

template<>
inline const char * name<navigation_interfaces::srv::SetNav_Request>()
{
  return "navigation_interfaces/srv/SetNav_Request";
}

template<>
struct has_fixed_size<navigation_interfaces::srv::SetNav_Request>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<navigation_interfaces::srv::SetNav_Request>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<navigation_interfaces::srv::SetNav_Request>
  : std::true_type {};

}  // namespace rosidl_generator_traits

namespace navigation_interfaces
{

namespace srv
{

inline void to_flow_style_yaml(
  const SetNav_Response & msg,
  std::ostream & out)
{
  out << "{";
  // member: returnedmessage
  {
    out << "returnedmessage: ";
    rosidl_generator_traits::value_to_yaml(msg.returnedmessage, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const SetNav_Response & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: returnedmessage
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "returnedmessage: ";
    rosidl_generator_traits::value_to_yaml(msg.returnedmessage, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const SetNav_Response & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace srv

}  // namespace navigation_interfaces

namespace rosidl_generator_traits
{

[[deprecated("use navigation_interfaces::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const navigation_interfaces::srv::SetNav_Response & msg,
  std::ostream & out, size_t indentation = 0)
{
  navigation_interfaces::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use navigation_interfaces::srv::to_yaml() instead")]]
inline std::string to_yaml(const navigation_interfaces::srv::SetNav_Response & msg)
{
  return navigation_interfaces::srv::to_yaml(msg);
}

template<>
inline const char * data_type<navigation_interfaces::srv::SetNav_Response>()
{
  return "navigation_interfaces::srv::SetNav_Response";
}

template<>
inline const char * name<navigation_interfaces::srv::SetNav_Response>()
{
  return "navigation_interfaces/srv/SetNav_Response";
}

template<>
struct has_fixed_size<navigation_interfaces::srv::SetNav_Response>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<navigation_interfaces::srv::SetNav_Response>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<navigation_interfaces::srv::SetNav_Response>
  : std::true_type {};

}  // namespace rosidl_generator_traits

namespace rosidl_generator_traits
{

template<>
inline const char * data_type<navigation_interfaces::srv::SetNav>()
{
  return "navigation_interfaces::srv::SetNav";
}

template<>
inline const char * name<navigation_interfaces::srv::SetNav>()
{
  return "navigation_interfaces/srv/SetNav";
}

template<>
struct has_fixed_size<navigation_interfaces::srv::SetNav>
  : std::integral_constant<
    bool,
    has_fixed_size<navigation_interfaces::srv::SetNav_Request>::value &&
    has_fixed_size<navigation_interfaces::srv::SetNav_Response>::value
  >
{
};

template<>
struct has_bounded_size<navigation_interfaces::srv::SetNav>
  : std::integral_constant<
    bool,
    has_bounded_size<navigation_interfaces::srv::SetNav_Request>::value &&
    has_bounded_size<navigation_interfaces::srv::SetNav_Response>::value
  >
{
};

template<>
struct is_service<navigation_interfaces::srv::SetNav>
  : std::true_type
{
};

template<>
struct is_service_request<navigation_interfaces::srv::SetNav_Request>
  : std::true_type
{
};

template<>
struct is_service_response<navigation_interfaces::srv::SetNav_Response>
  : std::true_type
{
};

}  // namespace rosidl_generator_traits

#endif  // NAVIGATION_INTERFACES__SRV__DETAIL__SET_NAV__TRAITS_HPP_

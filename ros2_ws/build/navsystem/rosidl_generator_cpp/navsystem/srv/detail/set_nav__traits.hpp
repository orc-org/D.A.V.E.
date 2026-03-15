// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from navsystem:srv/SetNav.idl
// generated code does not contain a copyright notice

#ifndef NAVSYSTEM__SRV__DETAIL__SET_NAV__TRAITS_HPP_
#define NAVSYSTEM__SRV__DETAIL__SET_NAV__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "navsystem/srv/detail/set_nav__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace navsystem
{

namespace srv
{

inline void to_flow_style_yaml(
  const SetNav_Request & msg,
  std::ostream & out)
{
  out << "{";
  // member: function
  {
    out << "function: ";
    rosidl_generator_traits::value_to_yaml(msg.function, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const SetNav_Request & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: function
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "function: ";
    rosidl_generator_traits::value_to_yaml(msg.function, out);
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

}  // namespace navsystem

namespace rosidl_generator_traits
{

[[deprecated("use navsystem::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const navsystem::srv::SetNav_Request & msg,
  std::ostream & out, size_t indentation = 0)
{
  navsystem::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use navsystem::srv::to_yaml() instead")]]
inline std::string to_yaml(const navsystem::srv::SetNav_Request & msg)
{
  return navsystem::srv::to_yaml(msg);
}

template<>
inline const char * data_type<navsystem::srv::SetNav_Request>()
{
  return "navsystem::srv::SetNav_Request";
}

template<>
inline const char * name<navsystem::srv::SetNav_Request>()
{
  return "navsystem/srv/SetNav_Request";
}

template<>
struct has_fixed_size<navsystem::srv::SetNav_Request>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<navsystem::srv::SetNav_Request>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<navsystem::srv::SetNav_Request>
  : std::true_type {};

}  // namespace rosidl_generator_traits

namespace navsystem
{

namespace srv
{

inline void to_flow_style_yaml(
  const SetNav_Response & msg,
  std::ostream & out)
{
  out << "{";
  // member: returned
  {
    out << "returned: ";
    rosidl_generator_traits::value_to_yaml(msg.returned, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const SetNav_Response & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: returned
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "returned: ";
    rosidl_generator_traits::value_to_yaml(msg.returned, out);
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

}  // namespace navsystem

namespace rosidl_generator_traits
{

[[deprecated("use navsystem::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const navsystem::srv::SetNav_Response & msg,
  std::ostream & out, size_t indentation = 0)
{
  navsystem::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use navsystem::srv::to_yaml() instead")]]
inline std::string to_yaml(const navsystem::srv::SetNav_Response & msg)
{
  return navsystem::srv::to_yaml(msg);
}

template<>
inline const char * data_type<navsystem::srv::SetNav_Response>()
{
  return "navsystem::srv::SetNav_Response";
}

template<>
inline const char * name<navsystem::srv::SetNav_Response>()
{
  return "navsystem/srv/SetNav_Response";
}

template<>
struct has_fixed_size<navsystem::srv::SetNav_Response>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<navsystem::srv::SetNav_Response>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<navsystem::srv::SetNav_Response>
  : std::true_type {};

}  // namespace rosidl_generator_traits

namespace rosidl_generator_traits
{

template<>
inline const char * data_type<navsystem::srv::SetNav>()
{
  return "navsystem::srv::SetNav";
}

template<>
inline const char * name<navsystem::srv::SetNav>()
{
  return "navsystem/srv/SetNav";
}

template<>
struct has_fixed_size<navsystem::srv::SetNav>
  : std::integral_constant<
    bool,
    has_fixed_size<navsystem::srv::SetNav_Request>::value &&
    has_fixed_size<navsystem::srv::SetNav_Response>::value
  >
{
};

template<>
struct has_bounded_size<navsystem::srv::SetNav>
  : std::integral_constant<
    bool,
    has_bounded_size<navsystem::srv::SetNav_Request>::value &&
    has_bounded_size<navsystem::srv::SetNav_Response>::value
  >
{
};

template<>
struct is_service<navsystem::srv::SetNav>
  : std::true_type
{
};

template<>
struct is_service_request<navsystem::srv::SetNav_Request>
  : std::true_type
{
};

template<>
struct is_service_response<navsystem::srv::SetNav_Response>
  : std::true_type
{
};

}  // namespace rosidl_generator_traits

#endif  // NAVSYSTEM__SRV__DETAIL__SET_NAV__TRAITS_HPP_

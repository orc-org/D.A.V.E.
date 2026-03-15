// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from navigation_interfaces:msg/Position.idl
// generated code does not contain a copyright notice

#ifndef NAVIGATION_INTERFACES__MSG__DETAIL__POSITION__TRAITS_HPP_
#define NAVIGATION_INTERFACES__MSG__DETAIL__POSITION__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "navigation_interfaces/msg/detail/position__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace navigation_interfaces
{

namespace msg
{

inline void to_flow_style_yaml(
  const Position & msg,
  std::ostream & out)
{
  out << "{";
  // member: position
  {
    out << "position: ";
    rosidl_generator_traits::value_to_yaml(msg.position, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const Position & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: position
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "position: ";
    rosidl_generator_traits::value_to_yaml(msg.position, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const Position & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace navigation_interfaces

namespace rosidl_generator_traits
{

[[deprecated("use navigation_interfaces::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const navigation_interfaces::msg::Position & msg,
  std::ostream & out, size_t indentation = 0)
{
  navigation_interfaces::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use navigation_interfaces::msg::to_yaml() instead")]]
inline std::string to_yaml(const navigation_interfaces::msg::Position & msg)
{
  return navigation_interfaces::msg::to_yaml(msg);
}

template<>
inline const char * data_type<navigation_interfaces::msg::Position>()
{
  return "navigation_interfaces::msg::Position";
}

template<>
inline const char * name<navigation_interfaces::msg::Position>()
{
  return "navigation_interfaces/msg/Position";
}

template<>
struct has_fixed_size<navigation_interfaces::msg::Position>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<navigation_interfaces::msg::Position>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<navigation_interfaces::msg::Position>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // NAVIGATION_INTERFACES__MSG__DETAIL__POSITION__TRAITS_HPP_

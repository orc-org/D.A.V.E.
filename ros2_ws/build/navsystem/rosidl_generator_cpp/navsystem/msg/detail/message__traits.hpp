// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from navsystem:msg/Message.idl
// generated code does not contain a copyright notice

#ifndef NAVSYSTEM__MSG__DETAIL__MESSAGE__TRAITS_HPP_
#define NAVSYSTEM__MSG__DETAIL__MESSAGE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "navsystem/msg/detail/message__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace navsystem
{

namespace msg
{

inline void to_flow_style_yaml(
  const Message & msg,
  std::ostream & out)
{
  out << "{";
  // member: message
  {
    out << "message: ";
    rosidl_generator_traits::value_to_yaml(msg.message, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const Message & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: message
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "message: ";
    rosidl_generator_traits::value_to_yaml(msg.message, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const Message & msg, bool use_flow_style = false)
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

}  // namespace navsystem

namespace rosidl_generator_traits
{

[[deprecated("use navsystem::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const navsystem::msg::Message & msg,
  std::ostream & out, size_t indentation = 0)
{
  navsystem::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use navsystem::msg::to_yaml() instead")]]
inline std::string to_yaml(const navsystem::msg::Message & msg)
{
  return navsystem::msg::to_yaml(msg);
}

template<>
inline const char * data_type<navsystem::msg::Message>()
{
  return "navsystem::msg::Message";
}

template<>
inline const char * name<navsystem::msg::Message>()
{
  return "navsystem/msg/Message";
}

template<>
struct has_fixed_size<navsystem::msg::Message>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<navsystem::msg::Message>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<navsystem::msg::Message>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // NAVSYSTEM__MSG__DETAIL__MESSAGE__TRAITS_HPP_

// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from dave_interfaces:msg/Bearing.idl
// generated code does not contain a copyright notice

#ifndef DAVE_INTERFACES__MSG__DETAIL__BEARING__BUILDER_HPP_
#define DAVE_INTERFACES__MSG__DETAIL__BEARING__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "dave_interfaces/msg/detail/bearing__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace dave_interfaces
{

namespace msg
{

namespace builder
{

class Init_Bearing_bearing
{
public:
  Init_Bearing_bearing()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::dave_interfaces::msg::Bearing bearing(::dave_interfaces::msg::Bearing::_bearing_type arg)
  {
    msg_.bearing = std::move(arg);
    return std::move(msg_);
  }

private:
  ::dave_interfaces::msg::Bearing msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::dave_interfaces::msg::Bearing>()
{
  return dave_interfaces::msg::builder::Init_Bearing_bearing();
}

}  // namespace dave_interfaces

#endif  // DAVE_INTERFACES__MSG__DETAIL__BEARING__BUILDER_HPP_

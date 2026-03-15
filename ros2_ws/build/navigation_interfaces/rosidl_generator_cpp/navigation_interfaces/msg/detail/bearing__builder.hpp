// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from navigation_interfaces:msg/Bearing.idl
// generated code does not contain a copyright notice

#ifndef NAVIGATION_INTERFACES__MSG__DETAIL__BEARING__BUILDER_HPP_
#define NAVIGATION_INTERFACES__MSG__DETAIL__BEARING__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "navigation_interfaces/msg/detail/bearing__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace navigation_interfaces
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
  ::navigation_interfaces::msg::Bearing bearing(::navigation_interfaces::msg::Bearing::_bearing_type arg)
  {
    msg_.bearing = std::move(arg);
    return std::move(msg_);
  }

private:
  ::navigation_interfaces::msg::Bearing msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::navigation_interfaces::msg::Bearing>()
{
  return navigation_interfaces::msg::builder::Init_Bearing_bearing();
}

}  // namespace navigation_interfaces

#endif  // NAVIGATION_INTERFACES__MSG__DETAIL__BEARING__BUILDER_HPP_

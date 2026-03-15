// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from custom_interfaces:srv/GetNav.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACES__SRV__DETAIL__GET_NAV__BUILDER_HPP_
#define CUSTOM_INTERFACES__SRV__DETAIL__GET_NAV__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "custom_interfaces/srv/detail/get_nav__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace custom_interfaces
{

namespace srv
{

namespace builder
{

class Init_GetNav_Request_function
{
public:
  Init_GetNav_Request_function()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interfaces::srv::GetNav_Request function(::custom_interfaces::srv::GetNav_Request::_function_type arg)
  {
    msg_.function = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::srv::GetNav_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::srv::GetNav_Request>()
{
  return custom_interfaces::srv::builder::Init_GetNav_Request_function();
}

}  // namespace custom_interfaces


namespace custom_interfaces
{

namespace srv
{

namespace builder
{

class Init_GetNav_Response_returned
{
public:
  Init_GetNav_Response_returned()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interfaces::srv::GetNav_Response returned(::custom_interfaces::srv::GetNav_Response::_returned_type arg)
  {
    msg_.returned = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::srv::GetNav_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::srv::GetNav_Response>()
{
  return custom_interfaces::srv::builder::Init_GetNav_Response_returned();
}

}  // namespace custom_interfaces

#endif  // CUSTOM_INTERFACES__SRV__DETAIL__GET_NAV__BUILDER_HPP_

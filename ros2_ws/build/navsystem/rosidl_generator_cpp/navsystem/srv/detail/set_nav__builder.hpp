// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from navsystem:srv/SetNav.idl
// generated code does not contain a copyright notice

#ifndef NAVSYSTEM__SRV__DETAIL__SET_NAV__BUILDER_HPP_
#define NAVSYSTEM__SRV__DETAIL__SET_NAV__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "navsystem/srv/detail/set_nav__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace navsystem
{

namespace srv
{

namespace builder
{

class Init_SetNav_Request_function
{
public:
  Init_SetNav_Request_function()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::navsystem::srv::SetNav_Request function(::navsystem::srv::SetNav_Request::_function_type arg)
  {
    msg_.function = std::move(arg);
    return std::move(msg_);
  }

private:
  ::navsystem::srv::SetNav_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::navsystem::srv::SetNav_Request>()
{
  return navsystem::srv::builder::Init_SetNav_Request_function();
}

}  // namespace navsystem


namespace navsystem
{

namespace srv
{

namespace builder
{

class Init_SetNav_Response_returned
{
public:
  Init_SetNav_Response_returned()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::navsystem::srv::SetNav_Response returned(::navsystem::srv::SetNav_Response::_returned_type arg)
  {
    msg_.returned = std::move(arg);
    return std::move(msg_);
  }

private:
  ::navsystem::srv::SetNav_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::navsystem::srv::SetNav_Response>()
{
  return navsystem::srv::builder::Init_SetNav_Response_returned();
}

}  // namespace navsystem

#endif  // NAVSYSTEM__SRV__DETAIL__SET_NAV__BUILDER_HPP_

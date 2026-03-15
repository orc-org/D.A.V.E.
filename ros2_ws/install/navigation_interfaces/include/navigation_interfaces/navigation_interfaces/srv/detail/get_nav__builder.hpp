// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from navigation_interfaces:srv/GetNav.idl
// generated code does not contain a copyright notice

#ifndef NAVIGATION_INTERFACES__SRV__DETAIL__GET_NAV__BUILDER_HPP_
#define NAVIGATION_INTERFACES__SRV__DETAIL__GET_NAV__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "navigation_interfaces/srv/detail/get_nav__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace navigation_interfaces
{

namespace srv
{

namespace builder
{

class Init_GetNav_Request_function2call
{
public:
  Init_GetNav_Request_function2call()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::navigation_interfaces::srv::GetNav_Request function2call(::navigation_interfaces::srv::GetNav_Request::_function2call_type arg)
  {
    msg_.function2call = std::move(arg);
    return std::move(msg_);
  }

private:
  ::navigation_interfaces::srv::GetNav_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::navigation_interfaces::srv::GetNav_Request>()
{
  return navigation_interfaces::srv::builder::Init_GetNav_Request_function2call();
}

}  // namespace navigation_interfaces


namespace navigation_interfaces
{

namespace srv
{

namespace builder
{

class Init_GetNav_Response_returnedmessage
{
public:
  Init_GetNav_Response_returnedmessage()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::navigation_interfaces::srv::GetNav_Response returnedmessage(::navigation_interfaces::srv::GetNav_Response::_returnedmessage_type arg)
  {
    msg_.returnedmessage = std::move(arg);
    return std::move(msg_);
  }

private:
  ::navigation_interfaces::srv::GetNav_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::navigation_interfaces::srv::GetNav_Response>()
{
  return navigation_interfaces::srv::builder::Init_GetNav_Response_returnedmessage();
}

}  // namespace navigation_interfaces

#endif  // NAVIGATION_INTERFACES__SRV__DETAIL__GET_NAV__BUILDER_HPP_

// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from navigation_interfaces:srv/SetNav.idl
// generated code does not contain a copyright notice

#ifndef NAVIGATION_INTERFACES__SRV__DETAIL__SET_NAV__BUILDER_HPP_
#define NAVIGATION_INTERFACES__SRV__DETAIL__SET_NAV__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "navigation_interfaces/srv/detail/set_nav__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace navigation_interfaces
{

namespace srv
{

namespace builder
{

class Init_SetNav_Request_arguments
{
public:
  explicit Init_SetNav_Request_arguments(::navigation_interfaces::srv::SetNav_Request & msg)
  : msg_(msg)
  {}
  ::navigation_interfaces::srv::SetNav_Request arguments(::navigation_interfaces::srv::SetNav_Request::_arguments_type arg)
  {
    msg_.arguments = std::move(arg);
    return std::move(msg_);
  }

private:
  ::navigation_interfaces::srv::SetNav_Request msg_;
};

class Init_SetNav_Request_function2call
{
public:
  Init_SetNav_Request_function2call()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_SetNav_Request_arguments function2call(::navigation_interfaces::srv::SetNav_Request::_function2call_type arg)
  {
    msg_.function2call = std::move(arg);
    return Init_SetNav_Request_arguments(msg_);
  }

private:
  ::navigation_interfaces::srv::SetNav_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::navigation_interfaces::srv::SetNav_Request>()
{
  return navigation_interfaces::srv::builder::Init_SetNav_Request_function2call();
}

}  // namespace navigation_interfaces


namespace navigation_interfaces
{

namespace srv
{

namespace builder
{

class Init_SetNav_Response_returnedmessage
{
public:
  Init_SetNav_Response_returnedmessage()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::navigation_interfaces::srv::SetNav_Response returnedmessage(::navigation_interfaces::srv::SetNav_Response::_returnedmessage_type arg)
  {
    msg_.returnedmessage = std::move(arg);
    return std::move(msg_);
  }

private:
  ::navigation_interfaces::srv::SetNav_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::navigation_interfaces::srv::SetNav_Response>()
{
  return navigation_interfaces::srv::builder::Init_SetNav_Response_returnedmessage();
}

}  // namespace navigation_interfaces

#endif  // NAVIGATION_INTERFACES__SRV__DETAIL__SET_NAV__BUILDER_HPP_

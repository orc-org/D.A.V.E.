// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from dave_interfaces:srv/GetSetNav.idl
// generated code does not contain a copyright notice

#ifndef DAVE_INTERFACES__SRV__DETAIL__GET_SET_NAV__BUILDER_HPP_
#define DAVE_INTERFACES__SRV__DETAIL__GET_SET_NAV__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "dave_interfaces/srv/detail/get_set_nav__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace dave_interfaces
{

namespace srv
{

namespace builder
{

class Init_GetSetNav_Request_function2call
{
public:
  Init_GetSetNav_Request_function2call()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::dave_interfaces::srv::GetSetNav_Request function2call(::dave_interfaces::srv::GetSetNav_Request::_function2call_type arg)
  {
    msg_.function2call = std::move(arg);
    return std::move(msg_);
  }

private:
  ::dave_interfaces::srv::GetSetNav_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::dave_interfaces::srv::GetSetNav_Request>()
{
  return dave_interfaces::srv::builder::Init_GetSetNav_Request_function2call();
}

}  // namespace dave_interfaces


namespace dave_interfaces
{

namespace srv
{

namespace builder
{

class Init_GetSetNav_Response_returnedmessage
{
public:
  Init_GetSetNav_Response_returnedmessage()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::dave_interfaces::srv::GetSetNav_Response returnedmessage(::dave_interfaces::srv::GetSetNav_Response::_returnedmessage_type arg)
  {
    msg_.returnedmessage = std::move(arg);
    return std::move(msg_);
  }

private:
  ::dave_interfaces::srv::GetSetNav_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::dave_interfaces::srv::GetSetNav_Response>()
{
  return dave_interfaces::srv::builder::Init_GetSetNav_Response_returnedmessage();
}

}  // namespace dave_interfaces

#endif  // DAVE_INTERFACES__SRV__DETAIL__GET_SET_NAV__BUILDER_HPP_

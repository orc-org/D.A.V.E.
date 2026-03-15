// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from navsystem:msg/Message.idl
// generated code does not contain a copyright notice

#ifndef NAVSYSTEM__MSG__DETAIL__MESSAGE__BUILDER_HPP_
#define NAVSYSTEM__MSG__DETAIL__MESSAGE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "navsystem/msg/detail/message__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace navsystem
{

namespace msg
{

namespace builder
{

class Init_Message_message
{
public:
  Init_Message_message()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::navsystem::msg::Message message(::navsystem::msg::Message::_message_type arg)
  {
    msg_.message = std::move(arg);
    return std::move(msg_);
  }

private:
  ::navsystem::msg::Message msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::navsystem::msg::Message>()
{
  return navsystem::msg::builder::Init_Message_message();
}

}  // namespace navsystem

#endif  // NAVSYSTEM__MSG__DETAIL__MESSAGE__BUILDER_HPP_

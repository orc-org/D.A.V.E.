// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from navsystem:srv/SetNav.idl
// generated code does not contain a copyright notice

#ifndef NAVSYSTEM__SRV__DETAIL__SET_NAV__STRUCT_HPP_
#define NAVSYSTEM__SRV__DETAIL__SET_NAV__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__navsystem__srv__SetNav_Request __attribute__((deprecated))
#else
# define DEPRECATED__navsystem__srv__SetNav_Request __declspec(deprecated)
#endif

namespace navsystem
{

namespace srv
{

// message struct
template<class ContainerAllocator>
struct SetNav_Request_
{
  using Type = SetNav_Request_<ContainerAllocator>;

  explicit SetNav_Request_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->function = "";
    }
  }

  explicit SetNav_Request_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : function(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->function = "";
    }
  }

  // field types and members
  using _function_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _function_type function;

  // setters for named parameter idiom
  Type & set__function(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->function = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    navsystem::srv::SetNav_Request_<ContainerAllocator> *;
  using ConstRawPtr =
    const navsystem::srv::SetNav_Request_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<navsystem::srv::SetNav_Request_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<navsystem::srv::SetNav_Request_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      navsystem::srv::SetNav_Request_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<navsystem::srv::SetNav_Request_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      navsystem::srv::SetNav_Request_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<navsystem::srv::SetNav_Request_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<navsystem::srv::SetNav_Request_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<navsystem::srv::SetNav_Request_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__navsystem__srv__SetNav_Request
    std::shared_ptr<navsystem::srv::SetNav_Request_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__navsystem__srv__SetNav_Request
    std::shared_ptr<navsystem::srv::SetNav_Request_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const SetNav_Request_ & other) const
  {
    if (this->function != other.function) {
      return false;
    }
    return true;
  }
  bool operator!=(const SetNav_Request_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct SetNav_Request_

// alias to use template instance with default allocator
using SetNav_Request =
  navsystem::srv::SetNav_Request_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace navsystem


#ifndef _WIN32
# define DEPRECATED__navsystem__srv__SetNav_Response __attribute__((deprecated))
#else
# define DEPRECATED__navsystem__srv__SetNav_Response __declspec(deprecated)
#endif

namespace navsystem
{

namespace srv
{

// message struct
template<class ContainerAllocator>
struct SetNav_Response_
{
  using Type = SetNav_Response_<ContainerAllocator>;

  explicit SetNav_Response_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->returned = "";
    }
  }

  explicit SetNav_Response_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : returned(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->returned = "";
    }
  }

  // field types and members
  using _returned_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _returned_type returned;

  // setters for named parameter idiom
  Type & set__returned(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->returned = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    navsystem::srv::SetNav_Response_<ContainerAllocator> *;
  using ConstRawPtr =
    const navsystem::srv::SetNav_Response_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<navsystem::srv::SetNav_Response_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<navsystem::srv::SetNav_Response_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      navsystem::srv::SetNav_Response_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<navsystem::srv::SetNav_Response_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      navsystem::srv::SetNav_Response_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<navsystem::srv::SetNav_Response_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<navsystem::srv::SetNav_Response_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<navsystem::srv::SetNav_Response_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__navsystem__srv__SetNav_Response
    std::shared_ptr<navsystem::srv::SetNav_Response_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__navsystem__srv__SetNav_Response
    std::shared_ptr<navsystem::srv::SetNav_Response_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const SetNav_Response_ & other) const
  {
    if (this->returned != other.returned) {
      return false;
    }
    return true;
  }
  bool operator!=(const SetNav_Response_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct SetNav_Response_

// alias to use template instance with default allocator
using SetNav_Response =
  navsystem::srv::SetNav_Response_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace navsystem

namespace navsystem
{

namespace srv
{

struct SetNav
{
  using Request = navsystem::srv::SetNav_Request;
  using Response = navsystem::srv::SetNav_Response;
};

}  // namespace srv

}  // namespace navsystem

#endif  // NAVSYSTEM__SRV__DETAIL__SET_NAV__STRUCT_HPP_

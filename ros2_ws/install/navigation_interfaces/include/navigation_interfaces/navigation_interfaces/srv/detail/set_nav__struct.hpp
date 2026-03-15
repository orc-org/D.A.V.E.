// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from navigation_interfaces:srv/SetNav.idl
// generated code does not contain a copyright notice

#ifndef NAVIGATION_INTERFACES__SRV__DETAIL__SET_NAV__STRUCT_HPP_
#define NAVIGATION_INTERFACES__SRV__DETAIL__SET_NAV__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__navigation_interfaces__srv__SetNav_Request __attribute__((deprecated))
#else
# define DEPRECATED__navigation_interfaces__srv__SetNav_Request __declspec(deprecated)
#endif

namespace navigation_interfaces
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
      this->function2call = "";
      this->arguments = "";
    }
  }

  explicit SetNav_Request_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : function2call(_alloc),
    arguments(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->function2call = "";
      this->arguments = "";
    }
  }

  // field types and members
  using _function2call_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _function2call_type function2call;
  using _arguments_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _arguments_type arguments;

  // setters for named parameter idiom
  Type & set__function2call(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->function2call = _arg;
    return *this;
  }
  Type & set__arguments(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->arguments = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    navigation_interfaces::srv::SetNav_Request_<ContainerAllocator> *;
  using ConstRawPtr =
    const navigation_interfaces::srv::SetNav_Request_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<navigation_interfaces::srv::SetNav_Request_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<navigation_interfaces::srv::SetNav_Request_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      navigation_interfaces::srv::SetNav_Request_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<navigation_interfaces::srv::SetNav_Request_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      navigation_interfaces::srv::SetNav_Request_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<navigation_interfaces::srv::SetNav_Request_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<navigation_interfaces::srv::SetNav_Request_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<navigation_interfaces::srv::SetNav_Request_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__navigation_interfaces__srv__SetNav_Request
    std::shared_ptr<navigation_interfaces::srv::SetNav_Request_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__navigation_interfaces__srv__SetNav_Request
    std::shared_ptr<navigation_interfaces::srv::SetNav_Request_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const SetNav_Request_ & other) const
  {
    if (this->function2call != other.function2call) {
      return false;
    }
    if (this->arguments != other.arguments) {
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
  navigation_interfaces::srv::SetNav_Request_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace navigation_interfaces


#ifndef _WIN32
# define DEPRECATED__navigation_interfaces__srv__SetNav_Response __attribute__((deprecated))
#else
# define DEPRECATED__navigation_interfaces__srv__SetNav_Response __declspec(deprecated)
#endif

namespace navigation_interfaces
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
      this->returnedmessage = "";
    }
  }

  explicit SetNav_Response_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : returnedmessage(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->returnedmessage = "";
    }
  }

  // field types and members
  using _returnedmessage_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _returnedmessage_type returnedmessage;

  // setters for named parameter idiom
  Type & set__returnedmessage(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->returnedmessage = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    navigation_interfaces::srv::SetNav_Response_<ContainerAllocator> *;
  using ConstRawPtr =
    const navigation_interfaces::srv::SetNav_Response_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<navigation_interfaces::srv::SetNav_Response_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<navigation_interfaces::srv::SetNav_Response_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      navigation_interfaces::srv::SetNav_Response_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<navigation_interfaces::srv::SetNav_Response_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      navigation_interfaces::srv::SetNav_Response_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<navigation_interfaces::srv::SetNav_Response_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<navigation_interfaces::srv::SetNav_Response_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<navigation_interfaces::srv::SetNav_Response_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__navigation_interfaces__srv__SetNav_Response
    std::shared_ptr<navigation_interfaces::srv::SetNav_Response_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__navigation_interfaces__srv__SetNav_Response
    std::shared_ptr<navigation_interfaces::srv::SetNav_Response_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const SetNav_Response_ & other) const
  {
    if (this->returnedmessage != other.returnedmessage) {
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
  navigation_interfaces::srv::SetNav_Response_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace navigation_interfaces

namespace navigation_interfaces
{

namespace srv
{

struct SetNav
{
  using Request = navigation_interfaces::srv::SetNav_Request;
  using Response = navigation_interfaces::srv::SetNav_Response;
};

}  // namespace srv

}  // namespace navigation_interfaces

#endif  // NAVIGATION_INTERFACES__SRV__DETAIL__SET_NAV__STRUCT_HPP_

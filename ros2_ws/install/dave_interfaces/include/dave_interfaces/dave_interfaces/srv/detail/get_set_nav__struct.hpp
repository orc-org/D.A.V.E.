// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from dave_interfaces:srv/GetSetNav.idl
// generated code does not contain a copyright notice

#ifndef DAVE_INTERFACES__SRV__DETAIL__GET_SET_NAV__STRUCT_HPP_
#define DAVE_INTERFACES__SRV__DETAIL__GET_SET_NAV__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__dave_interfaces__srv__GetSetNav_Request __attribute__((deprecated))
#else
# define DEPRECATED__dave_interfaces__srv__GetSetNav_Request __declspec(deprecated)
#endif

namespace dave_interfaces
{

namespace srv
{

// message struct
template<class ContainerAllocator>
struct GetSetNav_Request_
{
  using Type = GetSetNav_Request_<ContainerAllocator>;

  explicit GetSetNav_Request_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->function2call = "";
    }
  }

  explicit GetSetNav_Request_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : function2call(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->function2call = "";
    }
  }

  // field types and members
  using _function2call_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _function2call_type function2call;

  // setters for named parameter idiom
  Type & set__function2call(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->function2call = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator> *;
  using ConstRawPtr =
    const dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__dave_interfaces__srv__GetSetNav_Request
    std::shared_ptr<dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__dave_interfaces__srv__GetSetNav_Request
    std::shared_ptr<dave_interfaces::srv::GetSetNav_Request_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const GetSetNav_Request_ & other) const
  {
    if (this->function2call != other.function2call) {
      return false;
    }
    return true;
  }
  bool operator!=(const GetSetNav_Request_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct GetSetNav_Request_

// alias to use template instance with default allocator
using GetSetNav_Request =
  dave_interfaces::srv::GetSetNav_Request_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace dave_interfaces


#ifndef _WIN32
# define DEPRECATED__dave_interfaces__srv__GetSetNav_Response __attribute__((deprecated))
#else
# define DEPRECATED__dave_interfaces__srv__GetSetNav_Response __declspec(deprecated)
#endif

namespace dave_interfaces
{

namespace srv
{

// message struct
template<class ContainerAllocator>
struct GetSetNav_Response_
{
  using Type = GetSetNav_Response_<ContainerAllocator>;

  explicit GetSetNav_Response_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->returnedmessage = "";
    }
  }

  explicit GetSetNav_Response_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
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
    dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator> *;
  using ConstRawPtr =
    const dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__dave_interfaces__srv__GetSetNav_Response
    std::shared_ptr<dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__dave_interfaces__srv__GetSetNav_Response
    std::shared_ptr<dave_interfaces::srv::GetSetNav_Response_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const GetSetNav_Response_ & other) const
  {
    if (this->returnedmessage != other.returnedmessage) {
      return false;
    }
    return true;
  }
  bool operator!=(const GetSetNav_Response_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct GetSetNav_Response_

// alias to use template instance with default allocator
using GetSetNav_Response =
  dave_interfaces::srv::GetSetNav_Response_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace dave_interfaces

namespace dave_interfaces
{

namespace srv
{

struct GetSetNav
{
  using Request = dave_interfaces::srv::GetSetNav_Request;
  using Response = dave_interfaces::srv::GetSetNav_Response;
};

}  // namespace srv

}  // namespace dave_interfaces

#endif  // DAVE_INTERFACES__SRV__DETAIL__GET_SET_NAV__STRUCT_HPP_

// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from navigation_interfaces:msg/Bearing.idl
// generated code does not contain a copyright notice

#ifndef NAVIGATION_INTERFACES__MSG__DETAIL__BEARING__STRUCT_HPP_
#define NAVIGATION_INTERFACES__MSG__DETAIL__BEARING__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__navigation_interfaces__msg__Bearing __attribute__((deprecated))
#else
# define DEPRECATED__navigation_interfaces__msg__Bearing __declspec(deprecated)
#endif

namespace navigation_interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct Bearing_
{
  using Type = Bearing_<ContainerAllocator>;

  explicit Bearing_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->bearing = "";
    }
  }

  explicit Bearing_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : bearing(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->bearing = "";
    }
  }

  // field types and members
  using _bearing_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _bearing_type bearing;

  // setters for named parameter idiom
  Type & set__bearing(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->bearing = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    navigation_interfaces::msg::Bearing_<ContainerAllocator> *;
  using ConstRawPtr =
    const navigation_interfaces::msg::Bearing_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<navigation_interfaces::msg::Bearing_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<navigation_interfaces::msg::Bearing_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      navigation_interfaces::msg::Bearing_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<navigation_interfaces::msg::Bearing_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      navigation_interfaces::msg::Bearing_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<navigation_interfaces::msg::Bearing_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<navigation_interfaces::msg::Bearing_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<navigation_interfaces::msg::Bearing_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__navigation_interfaces__msg__Bearing
    std::shared_ptr<navigation_interfaces::msg::Bearing_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__navigation_interfaces__msg__Bearing
    std::shared_ptr<navigation_interfaces::msg::Bearing_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Bearing_ & other) const
  {
    if (this->bearing != other.bearing) {
      return false;
    }
    return true;
  }
  bool operator!=(const Bearing_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Bearing_

// alias to use template instance with default allocator
using Bearing =
  navigation_interfaces::msg::Bearing_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace navigation_interfaces

#endif  // NAVIGATION_INTERFACES__MSG__DETAIL__BEARING__STRUCT_HPP_

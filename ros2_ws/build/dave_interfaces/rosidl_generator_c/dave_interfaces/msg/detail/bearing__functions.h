// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from dave_interfaces:msg/Bearing.idl
// generated code does not contain a copyright notice

#ifndef DAVE_INTERFACES__MSG__DETAIL__BEARING__FUNCTIONS_H_
#define DAVE_INTERFACES__MSG__DETAIL__BEARING__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "dave_interfaces/msg/rosidl_generator_c__visibility_control.h"

#include "dave_interfaces/msg/detail/bearing__struct.h"

/// Initialize msg/Bearing message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * dave_interfaces__msg__Bearing
 * )) before or use
 * dave_interfaces__msg__Bearing__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__Bearing__init(dave_interfaces__msg__Bearing * msg);

/// Finalize msg/Bearing message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__msg__Bearing__fini(dave_interfaces__msg__Bearing * msg);

/// Create msg/Bearing message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * dave_interfaces__msg__Bearing__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
dave_interfaces__msg__Bearing *
dave_interfaces__msg__Bearing__create();

/// Destroy msg/Bearing message.
/**
 * It calls
 * dave_interfaces__msg__Bearing__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__msg__Bearing__destroy(dave_interfaces__msg__Bearing * msg);

/// Check for msg/Bearing message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__Bearing__are_equal(const dave_interfaces__msg__Bearing * lhs, const dave_interfaces__msg__Bearing * rhs);

/// Copy a msg/Bearing message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__Bearing__copy(
  const dave_interfaces__msg__Bearing * input,
  dave_interfaces__msg__Bearing * output);

/// Initialize array of msg/Bearing messages.
/**
 * It allocates the memory for the number of elements and calls
 * dave_interfaces__msg__Bearing__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__Bearing__Sequence__init(dave_interfaces__msg__Bearing__Sequence * array, size_t size);

/// Finalize array of msg/Bearing messages.
/**
 * It calls
 * dave_interfaces__msg__Bearing__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__msg__Bearing__Sequence__fini(dave_interfaces__msg__Bearing__Sequence * array);

/// Create array of msg/Bearing messages.
/**
 * It allocates the memory for the array and calls
 * dave_interfaces__msg__Bearing__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
dave_interfaces__msg__Bearing__Sequence *
dave_interfaces__msg__Bearing__Sequence__create(size_t size);

/// Destroy array of msg/Bearing messages.
/**
 * It calls
 * dave_interfaces__msg__Bearing__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__msg__Bearing__Sequence__destroy(dave_interfaces__msg__Bearing__Sequence * array);

/// Check for msg/Bearing message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__Bearing__Sequence__are_equal(const dave_interfaces__msg__Bearing__Sequence * lhs, const dave_interfaces__msg__Bearing__Sequence * rhs);

/// Copy an array of msg/Bearing messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__Bearing__Sequence__copy(
  const dave_interfaces__msg__Bearing__Sequence * input,
  dave_interfaces__msg__Bearing__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // DAVE_INTERFACES__MSG__DETAIL__BEARING__FUNCTIONS_H_

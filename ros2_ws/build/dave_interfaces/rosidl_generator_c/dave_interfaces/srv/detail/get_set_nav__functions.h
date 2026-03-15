// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from dave_interfaces:srv/GetSetNav.idl
// generated code does not contain a copyright notice

#ifndef DAVE_INTERFACES__SRV__DETAIL__GET_SET_NAV__FUNCTIONS_H_
#define DAVE_INTERFACES__SRV__DETAIL__GET_SET_NAV__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "dave_interfaces/msg/rosidl_generator_c__visibility_control.h"

#include "dave_interfaces/srv/detail/get_set_nav__struct.h"

/// Initialize srv/GetSetNav message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * dave_interfaces__srv__GetSetNav_Request
 * )) before or use
 * dave_interfaces__srv__GetSetNav_Request__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__srv__GetSetNav_Request__init(dave_interfaces__srv__GetSetNav_Request * msg);

/// Finalize srv/GetSetNav message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__srv__GetSetNav_Request__fini(dave_interfaces__srv__GetSetNav_Request * msg);

/// Create srv/GetSetNav message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * dave_interfaces__srv__GetSetNav_Request__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
dave_interfaces__srv__GetSetNav_Request *
dave_interfaces__srv__GetSetNav_Request__create();

/// Destroy srv/GetSetNav message.
/**
 * It calls
 * dave_interfaces__srv__GetSetNav_Request__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__srv__GetSetNav_Request__destroy(dave_interfaces__srv__GetSetNav_Request * msg);

/// Check for srv/GetSetNav message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__srv__GetSetNav_Request__are_equal(const dave_interfaces__srv__GetSetNav_Request * lhs, const dave_interfaces__srv__GetSetNav_Request * rhs);

/// Copy a srv/GetSetNav message.
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
dave_interfaces__srv__GetSetNav_Request__copy(
  const dave_interfaces__srv__GetSetNav_Request * input,
  dave_interfaces__srv__GetSetNav_Request * output);

/// Initialize array of srv/GetSetNav messages.
/**
 * It allocates the memory for the number of elements and calls
 * dave_interfaces__srv__GetSetNav_Request__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__srv__GetSetNav_Request__Sequence__init(dave_interfaces__srv__GetSetNav_Request__Sequence * array, size_t size);

/// Finalize array of srv/GetSetNav messages.
/**
 * It calls
 * dave_interfaces__srv__GetSetNav_Request__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__srv__GetSetNav_Request__Sequence__fini(dave_interfaces__srv__GetSetNav_Request__Sequence * array);

/// Create array of srv/GetSetNav messages.
/**
 * It allocates the memory for the array and calls
 * dave_interfaces__srv__GetSetNav_Request__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
dave_interfaces__srv__GetSetNav_Request__Sequence *
dave_interfaces__srv__GetSetNav_Request__Sequence__create(size_t size);

/// Destroy array of srv/GetSetNav messages.
/**
 * It calls
 * dave_interfaces__srv__GetSetNav_Request__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__srv__GetSetNav_Request__Sequence__destroy(dave_interfaces__srv__GetSetNav_Request__Sequence * array);

/// Check for srv/GetSetNav message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__srv__GetSetNav_Request__Sequence__are_equal(const dave_interfaces__srv__GetSetNav_Request__Sequence * lhs, const dave_interfaces__srv__GetSetNav_Request__Sequence * rhs);

/// Copy an array of srv/GetSetNav messages.
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
dave_interfaces__srv__GetSetNav_Request__Sequence__copy(
  const dave_interfaces__srv__GetSetNav_Request__Sequence * input,
  dave_interfaces__srv__GetSetNav_Request__Sequence * output);

/// Initialize srv/GetSetNav message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * dave_interfaces__srv__GetSetNav_Response
 * )) before or use
 * dave_interfaces__srv__GetSetNav_Response__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__srv__GetSetNav_Response__init(dave_interfaces__srv__GetSetNav_Response * msg);

/// Finalize srv/GetSetNav message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__srv__GetSetNav_Response__fini(dave_interfaces__srv__GetSetNav_Response * msg);

/// Create srv/GetSetNav message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * dave_interfaces__srv__GetSetNav_Response__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
dave_interfaces__srv__GetSetNav_Response *
dave_interfaces__srv__GetSetNav_Response__create();

/// Destroy srv/GetSetNav message.
/**
 * It calls
 * dave_interfaces__srv__GetSetNav_Response__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__srv__GetSetNav_Response__destroy(dave_interfaces__srv__GetSetNav_Response * msg);

/// Check for srv/GetSetNav message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__srv__GetSetNav_Response__are_equal(const dave_interfaces__srv__GetSetNav_Response * lhs, const dave_interfaces__srv__GetSetNav_Response * rhs);

/// Copy a srv/GetSetNav message.
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
dave_interfaces__srv__GetSetNav_Response__copy(
  const dave_interfaces__srv__GetSetNav_Response * input,
  dave_interfaces__srv__GetSetNav_Response * output);

/// Initialize array of srv/GetSetNav messages.
/**
 * It allocates the memory for the number of elements and calls
 * dave_interfaces__srv__GetSetNav_Response__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__srv__GetSetNav_Response__Sequence__init(dave_interfaces__srv__GetSetNav_Response__Sequence * array, size_t size);

/// Finalize array of srv/GetSetNav messages.
/**
 * It calls
 * dave_interfaces__srv__GetSetNav_Response__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__srv__GetSetNav_Response__Sequence__fini(dave_interfaces__srv__GetSetNav_Response__Sequence * array);

/// Create array of srv/GetSetNav messages.
/**
 * It allocates the memory for the array and calls
 * dave_interfaces__srv__GetSetNav_Response__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
dave_interfaces__srv__GetSetNav_Response__Sequence *
dave_interfaces__srv__GetSetNav_Response__Sequence__create(size_t size);

/// Destroy array of srv/GetSetNav messages.
/**
 * It calls
 * dave_interfaces__srv__GetSetNav_Response__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__srv__GetSetNav_Response__Sequence__destroy(dave_interfaces__srv__GetSetNav_Response__Sequence * array);

/// Check for srv/GetSetNav message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__srv__GetSetNav_Response__Sequence__are_equal(const dave_interfaces__srv__GetSetNav_Response__Sequence * lhs, const dave_interfaces__srv__GetSetNav_Response__Sequence * rhs);

/// Copy an array of srv/GetSetNav messages.
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
dave_interfaces__srv__GetSetNav_Response__Sequence__copy(
  const dave_interfaces__srv__GetSetNav_Response__Sequence * input,
  dave_interfaces__srv__GetSetNav_Response__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // DAVE_INTERFACES__SRV__DETAIL__GET_SET_NAV__FUNCTIONS_H_

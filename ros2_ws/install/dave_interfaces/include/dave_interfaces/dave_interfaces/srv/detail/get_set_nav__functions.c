// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from dave_interfaces:srv/GetSetNav.idl
// generated code does not contain a copyright notice
#include "dave_interfaces/srv/detail/get_set_nav__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"

// Include directives for member types
// Member `function2call`
#include "rosidl_runtime_c/string_functions.h"

bool
dave_interfaces__srv__GetSetNav_Request__init(dave_interfaces__srv__GetSetNav_Request * msg)
{
  if (!msg) {
    return false;
  }
  // function2call
  if (!rosidl_runtime_c__String__init(&msg->function2call)) {
    dave_interfaces__srv__GetSetNav_Request__fini(msg);
    return false;
  }
  return true;
}

void
dave_interfaces__srv__GetSetNav_Request__fini(dave_interfaces__srv__GetSetNav_Request * msg)
{
  if (!msg) {
    return;
  }
  // function2call
  rosidl_runtime_c__String__fini(&msg->function2call);
}

bool
dave_interfaces__srv__GetSetNav_Request__are_equal(const dave_interfaces__srv__GetSetNav_Request * lhs, const dave_interfaces__srv__GetSetNav_Request * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // function2call
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->function2call), &(rhs->function2call)))
  {
    return false;
  }
  return true;
}

bool
dave_interfaces__srv__GetSetNav_Request__copy(
  const dave_interfaces__srv__GetSetNav_Request * input,
  dave_interfaces__srv__GetSetNav_Request * output)
{
  if (!input || !output) {
    return false;
  }
  // function2call
  if (!rosidl_runtime_c__String__copy(
      &(input->function2call), &(output->function2call)))
  {
    return false;
  }
  return true;
}

dave_interfaces__srv__GetSetNav_Request *
dave_interfaces__srv__GetSetNav_Request__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__GetSetNav_Request * msg = (dave_interfaces__srv__GetSetNav_Request *)allocator.allocate(sizeof(dave_interfaces__srv__GetSetNav_Request), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(dave_interfaces__srv__GetSetNav_Request));
  bool success = dave_interfaces__srv__GetSetNav_Request__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
dave_interfaces__srv__GetSetNav_Request__destroy(dave_interfaces__srv__GetSetNav_Request * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    dave_interfaces__srv__GetSetNav_Request__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
dave_interfaces__srv__GetSetNav_Request__Sequence__init(dave_interfaces__srv__GetSetNav_Request__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__GetSetNav_Request * data = NULL;

  if (size) {
    data = (dave_interfaces__srv__GetSetNav_Request *)allocator.zero_allocate(size, sizeof(dave_interfaces__srv__GetSetNav_Request), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = dave_interfaces__srv__GetSetNav_Request__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        dave_interfaces__srv__GetSetNav_Request__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
dave_interfaces__srv__GetSetNav_Request__Sequence__fini(dave_interfaces__srv__GetSetNav_Request__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      dave_interfaces__srv__GetSetNav_Request__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

dave_interfaces__srv__GetSetNav_Request__Sequence *
dave_interfaces__srv__GetSetNav_Request__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__GetSetNav_Request__Sequence * array = (dave_interfaces__srv__GetSetNav_Request__Sequence *)allocator.allocate(sizeof(dave_interfaces__srv__GetSetNav_Request__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = dave_interfaces__srv__GetSetNav_Request__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
dave_interfaces__srv__GetSetNav_Request__Sequence__destroy(dave_interfaces__srv__GetSetNav_Request__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    dave_interfaces__srv__GetSetNav_Request__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
dave_interfaces__srv__GetSetNav_Request__Sequence__are_equal(const dave_interfaces__srv__GetSetNav_Request__Sequence * lhs, const dave_interfaces__srv__GetSetNav_Request__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!dave_interfaces__srv__GetSetNav_Request__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
dave_interfaces__srv__GetSetNav_Request__Sequence__copy(
  const dave_interfaces__srv__GetSetNav_Request__Sequence * input,
  dave_interfaces__srv__GetSetNav_Request__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(dave_interfaces__srv__GetSetNav_Request);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    dave_interfaces__srv__GetSetNav_Request * data =
      (dave_interfaces__srv__GetSetNav_Request *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!dave_interfaces__srv__GetSetNav_Request__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          dave_interfaces__srv__GetSetNav_Request__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!dave_interfaces__srv__GetSetNav_Request__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `returnedmessage`
// already included above
// #include "rosidl_runtime_c/string_functions.h"

bool
dave_interfaces__srv__GetSetNav_Response__init(dave_interfaces__srv__GetSetNav_Response * msg)
{
  if (!msg) {
    return false;
  }
  // returnedmessage
  if (!rosidl_runtime_c__String__init(&msg->returnedmessage)) {
    dave_interfaces__srv__GetSetNav_Response__fini(msg);
    return false;
  }
  return true;
}

void
dave_interfaces__srv__GetSetNav_Response__fini(dave_interfaces__srv__GetSetNav_Response * msg)
{
  if (!msg) {
    return;
  }
  // returnedmessage
  rosidl_runtime_c__String__fini(&msg->returnedmessage);
}

bool
dave_interfaces__srv__GetSetNav_Response__are_equal(const dave_interfaces__srv__GetSetNav_Response * lhs, const dave_interfaces__srv__GetSetNav_Response * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // returnedmessage
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->returnedmessage), &(rhs->returnedmessage)))
  {
    return false;
  }
  return true;
}

bool
dave_interfaces__srv__GetSetNav_Response__copy(
  const dave_interfaces__srv__GetSetNav_Response * input,
  dave_interfaces__srv__GetSetNav_Response * output)
{
  if (!input || !output) {
    return false;
  }
  // returnedmessage
  if (!rosidl_runtime_c__String__copy(
      &(input->returnedmessage), &(output->returnedmessage)))
  {
    return false;
  }
  return true;
}

dave_interfaces__srv__GetSetNav_Response *
dave_interfaces__srv__GetSetNav_Response__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__GetSetNav_Response * msg = (dave_interfaces__srv__GetSetNav_Response *)allocator.allocate(sizeof(dave_interfaces__srv__GetSetNav_Response), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(dave_interfaces__srv__GetSetNav_Response));
  bool success = dave_interfaces__srv__GetSetNav_Response__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
dave_interfaces__srv__GetSetNav_Response__destroy(dave_interfaces__srv__GetSetNav_Response * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    dave_interfaces__srv__GetSetNav_Response__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
dave_interfaces__srv__GetSetNav_Response__Sequence__init(dave_interfaces__srv__GetSetNav_Response__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__GetSetNav_Response * data = NULL;

  if (size) {
    data = (dave_interfaces__srv__GetSetNav_Response *)allocator.zero_allocate(size, sizeof(dave_interfaces__srv__GetSetNav_Response), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = dave_interfaces__srv__GetSetNav_Response__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        dave_interfaces__srv__GetSetNav_Response__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
dave_interfaces__srv__GetSetNav_Response__Sequence__fini(dave_interfaces__srv__GetSetNav_Response__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      dave_interfaces__srv__GetSetNav_Response__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

dave_interfaces__srv__GetSetNav_Response__Sequence *
dave_interfaces__srv__GetSetNav_Response__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__GetSetNav_Response__Sequence * array = (dave_interfaces__srv__GetSetNav_Response__Sequence *)allocator.allocate(sizeof(dave_interfaces__srv__GetSetNav_Response__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = dave_interfaces__srv__GetSetNav_Response__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
dave_interfaces__srv__GetSetNav_Response__Sequence__destroy(dave_interfaces__srv__GetSetNav_Response__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    dave_interfaces__srv__GetSetNav_Response__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
dave_interfaces__srv__GetSetNav_Response__Sequence__are_equal(const dave_interfaces__srv__GetSetNav_Response__Sequence * lhs, const dave_interfaces__srv__GetSetNav_Response__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!dave_interfaces__srv__GetSetNav_Response__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
dave_interfaces__srv__GetSetNav_Response__Sequence__copy(
  const dave_interfaces__srv__GetSetNav_Response__Sequence * input,
  dave_interfaces__srv__GetSetNav_Response__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(dave_interfaces__srv__GetSetNav_Response);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    dave_interfaces__srv__GetSetNav_Response * data =
      (dave_interfaces__srv__GetSetNav_Response *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!dave_interfaces__srv__GetSetNav_Response__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          dave_interfaces__srv__GetSetNav_Response__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!dave_interfaces__srv__GetSetNav_Response__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}

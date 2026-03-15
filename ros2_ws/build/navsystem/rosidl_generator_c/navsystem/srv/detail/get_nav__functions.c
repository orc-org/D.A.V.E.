// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from navsystem:srv/GetNav.idl
// generated code does not contain a copyright notice
#include "navsystem/srv/detail/get_nav__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"

// Include directives for member types
// Member `function`
#include "rosidl_runtime_c/string_functions.h"

bool
navsystem__srv__GetNav_Request__init(navsystem__srv__GetNav_Request * msg)
{
  if (!msg) {
    return false;
  }
  // function
  if (!rosidl_runtime_c__String__init(&msg->function)) {
    navsystem__srv__GetNav_Request__fini(msg);
    return false;
  }
  return true;
}

void
navsystem__srv__GetNav_Request__fini(navsystem__srv__GetNav_Request * msg)
{
  if (!msg) {
    return;
  }
  // function
  rosidl_runtime_c__String__fini(&msg->function);
}

bool
navsystem__srv__GetNav_Request__are_equal(const navsystem__srv__GetNav_Request * lhs, const navsystem__srv__GetNav_Request * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // function
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->function), &(rhs->function)))
  {
    return false;
  }
  return true;
}

bool
navsystem__srv__GetNav_Request__copy(
  const navsystem__srv__GetNav_Request * input,
  navsystem__srv__GetNav_Request * output)
{
  if (!input || !output) {
    return false;
  }
  // function
  if (!rosidl_runtime_c__String__copy(
      &(input->function), &(output->function)))
  {
    return false;
  }
  return true;
}

navsystem__srv__GetNav_Request *
navsystem__srv__GetNav_Request__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navsystem__srv__GetNav_Request * msg = (navsystem__srv__GetNav_Request *)allocator.allocate(sizeof(navsystem__srv__GetNav_Request), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(navsystem__srv__GetNav_Request));
  bool success = navsystem__srv__GetNav_Request__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
navsystem__srv__GetNav_Request__destroy(navsystem__srv__GetNav_Request * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    navsystem__srv__GetNav_Request__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
navsystem__srv__GetNav_Request__Sequence__init(navsystem__srv__GetNav_Request__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navsystem__srv__GetNav_Request * data = NULL;

  if (size) {
    data = (navsystem__srv__GetNav_Request *)allocator.zero_allocate(size, sizeof(navsystem__srv__GetNav_Request), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = navsystem__srv__GetNav_Request__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        navsystem__srv__GetNav_Request__fini(&data[i - 1]);
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
navsystem__srv__GetNav_Request__Sequence__fini(navsystem__srv__GetNav_Request__Sequence * array)
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
      navsystem__srv__GetNav_Request__fini(&array->data[i]);
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

navsystem__srv__GetNav_Request__Sequence *
navsystem__srv__GetNav_Request__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navsystem__srv__GetNav_Request__Sequence * array = (navsystem__srv__GetNav_Request__Sequence *)allocator.allocate(sizeof(navsystem__srv__GetNav_Request__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = navsystem__srv__GetNav_Request__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
navsystem__srv__GetNav_Request__Sequence__destroy(navsystem__srv__GetNav_Request__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    navsystem__srv__GetNav_Request__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
navsystem__srv__GetNav_Request__Sequence__are_equal(const navsystem__srv__GetNav_Request__Sequence * lhs, const navsystem__srv__GetNav_Request__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!navsystem__srv__GetNav_Request__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
navsystem__srv__GetNav_Request__Sequence__copy(
  const navsystem__srv__GetNav_Request__Sequence * input,
  navsystem__srv__GetNav_Request__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(navsystem__srv__GetNav_Request);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    navsystem__srv__GetNav_Request * data =
      (navsystem__srv__GetNav_Request *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!navsystem__srv__GetNav_Request__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          navsystem__srv__GetNav_Request__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!navsystem__srv__GetNav_Request__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `returned`
// already included above
// #include "rosidl_runtime_c/string_functions.h"

bool
navsystem__srv__GetNav_Response__init(navsystem__srv__GetNav_Response * msg)
{
  if (!msg) {
    return false;
  }
  // returned
  if (!rosidl_runtime_c__String__init(&msg->returned)) {
    navsystem__srv__GetNav_Response__fini(msg);
    return false;
  }
  return true;
}

void
navsystem__srv__GetNav_Response__fini(navsystem__srv__GetNav_Response * msg)
{
  if (!msg) {
    return;
  }
  // returned
  rosidl_runtime_c__String__fini(&msg->returned);
}

bool
navsystem__srv__GetNav_Response__are_equal(const navsystem__srv__GetNav_Response * lhs, const navsystem__srv__GetNav_Response * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // returned
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->returned), &(rhs->returned)))
  {
    return false;
  }
  return true;
}

bool
navsystem__srv__GetNav_Response__copy(
  const navsystem__srv__GetNav_Response * input,
  navsystem__srv__GetNav_Response * output)
{
  if (!input || !output) {
    return false;
  }
  // returned
  if (!rosidl_runtime_c__String__copy(
      &(input->returned), &(output->returned)))
  {
    return false;
  }
  return true;
}

navsystem__srv__GetNav_Response *
navsystem__srv__GetNav_Response__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navsystem__srv__GetNav_Response * msg = (navsystem__srv__GetNav_Response *)allocator.allocate(sizeof(navsystem__srv__GetNav_Response), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(navsystem__srv__GetNav_Response));
  bool success = navsystem__srv__GetNav_Response__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
navsystem__srv__GetNav_Response__destroy(navsystem__srv__GetNav_Response * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    navsystem__srv__GetNav_Response__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
navsystem__srv__GetNav_Response__Sequence__init(navsystem__srv__GetNav_Response__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navsystem__srv__GetNav_Response * data = NULL;

  if (size) {
    data = (navsystem__srv__GetNav_Response *)allocator.zero_allocate(size, sizeof(navsystem__srv__GetNav_Response), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = navsystem__srv__GetNav_Response__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        navsystem__srv__GetNav_Response__fini(&data[i - 1]);
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
navsystem__srv__GetNav_Response__Sequence__fini(navsystem__srv__GetNav_Response__Sequence * array)
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
      navsystem__srv__GetNav_Response__fini(&array->data[i]);
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

navsystem__srv__GetNav_Response__Sequence *
navsystem__srv__GetNav_Response__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navsystem__srv__GetNav_Response__Sequence * array = (navsystem__srv__GetNav_Response__Sequence *)allocator.allocate(sizeof(navsystem__srv__GetNav_Response__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = navsystem__srv__GetNav_Response__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
navsystem__srv__GetNav_Response__Sequence__destroy(navsystem__srv__GetNav_Response__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    navsystem__srv__GetNav_Response__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
navsystem__srv__GetNav_Response__Sequence__are_equal(const navsystem__srv__GetNav_Response__Sequence * lhs, const navsystem__srv__GetNav_Response__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!navsystem__srv__GetNav_Response__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
navsystem__srv__GetNav_Response__Sequence__copy(
  const navsystem__srv__GetNav_Response__Sequence * input,
  navsystem__srv__GetNav_Response__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(navsystem__srv__GetNav_Response);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    navsystem__srv__GetNav_Response * data =
      (navsystem__srv__GetNav_Response *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!navsystem__srv__GetNav_Response__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          navsystem__srv__GetNav_Response__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!navsystem__srv__GetNav_Response__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}

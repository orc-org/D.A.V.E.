// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from navsystem:msg/Message.idl
// generated code does not contain a copyright notice
#include "navsystem/msg/detail/message__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `message`
#include "rosidl_runtime_c/string_functions.h"

bool
navsystem__msg__Message__init(navsystem__msg__Message * msg)
{
  if (!msg) {
    return false;
  }
  // message
  if (!rosidl_runtime_c__String__init(&msg->message)) {
    navsystem__msg__Message__fini(msg);
    return false;
  }
  return true;
}

void
navsystem__msg__Message__fini(navsystem__msg__Message * msg)
{
  if (!msg) {
    return;
  }
  // message
  rosidl_runtime_c__String__fini(&msg->message);
}

bool
navsystem__msg__Message__are_equal(const navsystem__msg__Message * lhs, const navsystem__msg__Message * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // message
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->message), &(rhs->message)))
  {
    return false;
  }
  return true;
}

bool
navsystem__msg__Message__copy(
  const navsystem__msg__Message * input,
  navsystem__msg__Message * output)
{
  if (!input || !output) {
    return false;
  }
  // message
  if (!rosidl_runtime_c__String__copy(
      &(input->message), &(output->message)))
  {
    return false;
  }
  return true;
}

navsystem__msg__Message *
navsystem__msg__Message__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navsystem__msg__Message * msg = (navsystem__msg__Message *)allocator.allocate(sizeof(navsystem__msg__Message), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(navsystem__msg__Message));
  bool success = navsystem__msg__Message__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
navsystem__msg__Message__destroy(navsystem__msg__Message * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    navsystem__msg__Message__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
navsystem__msg__Message__Sequence__init(navsystem__msg__Message__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navsystem__msg__Message * data = NULL;

  if (size) {
    data = (navsystem__msg__Message *)allocator.zero_allocate(size, sizeof(navsystem__msg__Message), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = navsystem__msg__Message__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        navsystem__msg__Message__fini(&data[i - 1]);
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
navsystem__msg__Message__Sequence__fini(navsystem__msg__Message__Sequence * array)
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
      navsystem__msg__Message__fini(&array->data[i]);
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

navsystem__msg__Message__Sequence *
navsystem__msg__Message__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navsystem__msg__Message__Sequence * array = (navsystem__msg__Message__Sequence *)allocator.allocate(sizeof(navsystem__msg__Message__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = navsystem__msg__Message__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
navsystem__msg__Message__Sequence__destroy(navsystem__msg__Message__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    navsystem__msg__Message__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
navsystem__msg__Message__Sequence__are_equal(const navsystem__msg__Message__Sequence * lhs, const navsystem__msg__Message__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!navsystem__msg__Message__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
navsystem__msg__Message__Sequence__copy(
  const navsystem__msg__Message__Sequence * input,
  navsystem__msg__Message__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(navsystem__msg__Message);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    navsystem__msg__Message * data =
      (navsystem__msg__Message *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!navsystem__msg__Message__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          navsystem__msg__Message__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!navsystem__msg__Message__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}

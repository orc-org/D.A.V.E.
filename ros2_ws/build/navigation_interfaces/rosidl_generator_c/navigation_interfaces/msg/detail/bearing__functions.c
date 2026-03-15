// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from navigation_interfaces:msg/Bearing.idl
// generated code does not contain a copyright notice
#include "navigation_interfaces/msg/detail/bearing__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `bearing`
#include "rosidl_runtime_c/string_functions.h"

bool
navigation_interfaces__msg__Bearing__init(navigation_interfaces__msg__Bearing * msg)
{
  if (!msg) {
    return false;
  }
  // bearing
  if (!rosidl_runtime_c__String__init(&msg->bearing)) {
    navigation_interfaces__msg__Bearing__fini(msg);
    return false;
  }
  return true;
}

void
navigation_interfaces__msg__Bearing__fini(navigation_interfaces__msg__Bearing * msg)
{
  if (!msg) {
    return;
  }
  // bearing
  rosidl_runtime_c__String__fini(&msg->bearing);
}

bool
navigation_interfaces__msg__Bearing__are_equal(const navigation_interfaces__msg__Bearing * lhs, const navigation_interfaces__msg__Bearing * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // bearing
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->bearing), &(rhs->bearing)))
  {
    return false;
  }
  return true;
}

bool
navigation_interfaces__msg__Bearing__copy(
  const navigation_interfaces__msg__Bearing * input,
  navigation_interfaces__msg__Bearing * output)
{
  if (!input || !output) {
    return false;
  }
  // bearing
  if (!rosidl_runtime_c__String__copy(
      &(input->bearing), &(output->bearing)))
  {
    return false;
  }
  return true;
}

navigation_interfaces__msg__Bearing *
navigation_interfaces__msg__Bearing__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navigation_interfaces__msg__Bearing * msg = (navigation_interfaces__msg__Bearing *)allocator.allocate(sizeof(navigation_interfaces__msg__Bearing), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(navigation_interfaces__msg__Bearing));
  bool success = navigation_interfaces__msg__Bearing__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
navigation_interfaces__msg__Bearing__destroy(navigation_interfaces__msg__Bearing * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    navigation_interfaces__msg__Bearing__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
navigation_interfaces__msg__Bearing__Sequence__init(navigation_interfaces__msg__Bearing__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navigation_interfaces__msg__Bearing * data = NULL;

  if (size) {
    data = (navigation_interfaces__msg__Bearing *)allocator.zero_allocate(size, sizeof(navigation_interfaces__msg__Bearing), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = navigation_interfaces__msg__Bearing__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        navigation_interfaces__msg__Bearing__fini(&data[i - 1]);
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
navigation_interfaces__msg__Bearing__Sequence__fini(navigation_interfaces__msg__Bearing__Sequence * array)
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
      navigation_interfaces__msg__Bearing__fini(&array->data[i]);
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

navigation_interfaces__msg__Bearing__Sequence *
navigation_interfaces__msg__Bearing__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  navigation_interfaces__msg__Bearing__Sequence * array = (navigation_interfaces__msg__Bearing__Sequence *)allocator.allocate(sizeof(navigation_interfaces__msg__Bearing__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = navigation_interfaces__msg__Bearing__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
navigation_interfaces__msg__Bearing__Sequence__destroy(navigation_interfaces__msg__Bearing__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    navigation_interfaces__msg__Bearing__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
navigation_interfaces__msg__Bearing__Sequence__are_equal(const navigation_interfaces__msg__Bearing__Sequence * lhs, const navigation_interfaces__msg__Bearing__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!navigation_interfaces__msg__Bearing__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
navigation_interfaces__msg__Bearing__Sequence__copy(
  const navigation_interfaces__msg__Bearing__Sequence * input,
  navigation_interfaces__msg__Bearing__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(navigation_interfaces__msg__Bearing);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    navigation_interfaces__msg__Bearing * data =
      (navigation_interfaces__msg__Bearing *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!navigation_interfaces__msg__Bearing__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          navigation_interfaces__msg__Bearing__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!navigation_interfaces__msg__Bearing__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}

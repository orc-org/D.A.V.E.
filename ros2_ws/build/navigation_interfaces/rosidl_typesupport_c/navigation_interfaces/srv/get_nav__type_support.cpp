// generated from rosidl_typesupport_c/resource/idl__type_support.cpp.em
// with input from navigation_interfaces:srv/GetNav.idl
// generated code does not contain a copyright notice

#include "cstddef"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "navigation_interfaces/srv/detail/get_nav__struct.h"
#include "navigation_interfaces/srv/detail/get_nav__type_support.h"
#include "rosidl_typesupport_c/identifier.h"
#include "rosidl_typesupport_c/message_type_support_dispatch.h"
#include "rosidl_typesupport_c/type_support_map.h"
#include "rosidl_typesupport_c/visibility_control.h"
#include "rosidl_typesupport_interface/macros.h"

namespace navigation_interfaces
{

namespace srv
{

namespace rosidl_typesupport_c
{

typedef struct _GetNav_Request_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _GetNav_Request_type_support_ids_t;

static const _GetNav_Request_type_support_ids_t _GetNav_Request_message_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_c",  // ::rosidl_typesupport_fastrtps_c::typesupport_identifier,
    "rosidl_typesupport_introspection_c",  // ::rosidl_typesupport_introspection_c::typesupport_identifier,
  }
};

typedef struct _GetNav_Request_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _GetNav_Request_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _GetNav_Request_type_support_symbol_names_t _GetNav_Request_message_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, navigation_interfaces, srv, GetNav_Request)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, navigation_interfaces, srv, GetNav_Request)),
  }
};

typedef struct _GetNav_Request_type_support_data_t
{
  void * data[2];
} _GetNav_Request_type_support_data_t;

static _GetNav_Request_type_support_data_t _GetNav_Request_message_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _GetNav_Request_message_typesupport_map = {
  2,
  "navigation_interfaces",
  &_GetNav_Request_message_typesupport_ids.typesupport_identifier[0],
  &_GetNav_Request_message_typesupport_symbol_names.symbol_name[0],
  &_GetNav_Request_message_typesupport_data.data[0],
};

static const rosidl_message_type_support_t GetNav_Request_message_type_support_handle = {
  rosidl_typesupport_c__typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_GetNav_Request_message_typesupport_map),
  rosidl_typesupport_c__get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_c

}  // namespace srv

}  // namespace navigation_interfaces

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_c, navigation_interfaces, srv, GetNav_Request)() {
  return &::navigation_interfaces::srv::rosidl_typesupport_c::GetNav_Request_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif

// already included above
// #include "cstddef"
// already included above
// #include "rosidl_runtime_c/message_type_support_struct.h"
// already included above
// #include "navigation_interfaces/srv/detail/get_nav__struct.h"
// already included above
// #include "navigation_interfaces/srv/detail/get_nav__type_support.h"
// already included above
// #include "rosidl_typesupport_c/identifier.h"
// already included above
// #include "rosidl_typesupport_c/message_type_support_dispatch.h"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
// already included above
// #include "rosidl_typesupport_c/visibility_control.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace navigation_interfaces
{

namespace srv
{

namespace rosidl_typesupport_c
{

typedef struct _GetNav_Response_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _GetNav_Response_type_support_ids_t;

static const _GetNav_Response_type_support_ids_t _GetNav_Response_message_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_c",  // ::rosidl_typesupport_fastrtps_c::typesupport_identifier,
    "rosidl_typesupport_introspection_c",  // ::rosidl_typesupport_introspection_c::typesupport_identifier,
  }
};

typedef struct _GetNav_Response_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _GetNav_Response_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _GetNav_Response_type_support_symbol_names_t _GetNav_Response_message_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, navigation_interfaces, srv, GetNav_Response)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, navigation_interfaces, srv, GetNav_Response)),
  }
};

typedef struct _GetNav_Response_type_support_data_t
{
  void * data[2];
} _GetNav_Response_type_support_data_t;

static _GetNav_Response_type_support_data_t _GetNav_Response_message_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _GetNav_Response_message_typesupport_map = {
  2,
  "navigation_interfaces",
  &_GetNav_Response_message_typesupport_ids.typesupport_identifier[0],
  &_GetNav_Response_message_typesupport_symbol_names.symbol_name[0],
  &_GetNav_Response_message_typesupport_data.data[0],
};

static const rosidl_message_type_support_t GetNav_Response_message_type_support_handle = {
  rosidl_typesupport_c__typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_GetNav_Response_message_typesupport_map),
  rosidl_typesupport_c__get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_c

}  // namespace srv

}  // namespace navigation_interfaces

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_c, navigation_interfaces, srv, GetNav_Response)() {
  return &::navigation_interfaces::srv::rosidl_typesupport_c::GetNav_Response_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif

// already included above
// #include "cstddef"
#include "rosidl_runtime_c/service_type_support_struct.h"
// already included above
// #include "navigation_interfaces/srv/detail/get_nav__type_support.h"
// already included above
// #include "rosidl_typesupport_c/identifier.h"
#include "rosidl_typesupport_c/service_type_support_dispatch.h"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace navigation_interfaces
{

namespace srv
{

namespace rosidl_typesupport_c
{

typedef struct _GetNav_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _GetNav_type_support_ids_t;

static const _GetNav_type_support_ids_t _GetNav_service_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_c",  // ::rosidl_typesupport_fastrtps_c::typesupport_identifier,
    "rosidl_typesupport_introspection_c",  // ::rosidl_typesupport_introspection_c::typesupport_identifier,
  }
};

typedef struct _GetNav_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _GetNav_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _GetNav_type_support_symbol_names_t _GetNav_service_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, navigation_interfaces, srv, GetNav)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_introspection_c, navigation_interfaces, srv, GetNav)),
  }
};

typedef struct _GetNav_type_support_data_t
{
  void * data[2];
} _GetNav_type_support_data_t;

static _GetNav_type_support_data_t _GetNav_service_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _GetNav_service_typesupport_map = {
  2,
  "navigation_interfaces",
  &_GetNav_service_typesupport_ids.typesupport_identifier[0],
  &_GetNav_service_typesupport_symbol_names.symbol_name[0],
  &_GetNav_service_typesupport_data.data[0],
};

static const rosidl_service_type_support_t GetNav_service_type_support_handle = {
  rosidl_typesupport_c__typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_GetNav_service_typesupport_map),
  rosidl_typesupport_c__get_service_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_c

}  // namespace srv

}  // namespace navigation_interfaces

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_c, navigation_interfaces, srv, GetNav)() {
  return &::navigation_interfaces::srv::rosidl_typesupport_c::GetNav_service_type_support_handle;
}

#ifdef __cplusplus
}
#endif

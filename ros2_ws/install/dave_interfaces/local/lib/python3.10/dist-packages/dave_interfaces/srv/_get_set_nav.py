# generated from rosidl_generator_py/resource/_idl.py.em
# with input from dave_interfaces:srv/GetSetNav.idl
# generated code does not contain a copyright notice


# Import statements for member types

import builtins  # noqa: E402, I100

import rosidl_parser.definition  # noqa: E402, I100


class Metaclass_GetSetNav_Request(type):
    """Metaclass of message 'GetSetNav_Request'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('dave_interfaces')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'dave_interfaces.srv.GetSetNav_Request')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__srv__get_set_nav__request
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__srv__get_set_nav__request
            cls._CONVERT_TO_PY = module.convert_to_py_msg__srv__get_set_nav__request
            cls._TYPE_SUPPORT = module.type_support_msg__srv__get_set_nav__request
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__srv__get_set_nav__request

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class GetSetNav_Request(metaclass=Metaclass_GetSetNav_Request):
    """Message class 'GetSetNav_Request'."""

    __slots__ = [
        '_function2call',
    ]

    _fields_and_field_types = {
        'function2call': 'string',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.UnboundedString(),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.function2call = kwargs.get('function2call', str())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.function2call != other.function2call:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def function2call(self):
        """Message field 'function2call'."""
        return self._function2call

    @function2call.setter
    def function2call(self, value):
        if __debug__:
            assert \
                isinstance(value, str), \
                "The 'function2call' field must be of type 'str'"
        self._function2call = value


# Import statements for member types

# already imported above
# import builtins

# already imported above
# import rosidl_parser.definition


class Metaclass_GetSetNav_Response(type):
    """Metaclass of message 'GetSetNav_Response'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('dave_interfaces')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'dave_interfaces.srv.GetSetNav_Response')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__srv__get_set_nav__response
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__srv__get_set_nav__response
            cls._CONVERT_TO_PY = module.convert_to_py_msg__srv__get_set_nav__response
            cls._TYPE_SUPPORT = module.type_support_msg__srv__get_set_nav__response
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__srv__get_set_nav__response

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class GetSetNav_Response(metaclass=Metaclass_GetSetNav_Response):
    """Message class 'GetSetNav_Response'."""

    __slots__ = [
        '_returnedmessage',
    ]

    _fields_and_field_types = {
        'returnedmessage': 'string',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.UnboundedString(),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.returnedmessage = kwargs.get('returnedmessage', str())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.returnedmessage != other.returnedmessage:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def returnedmessage(self):
        """Message field 'returnedmessage'."""
        return self._returnedmessage

    @returnedmessage.setter
    def returnedmessage(self, value):
        if __debug__:
            assert \
                isinstance(value, str), \
                "The 'returnedmessage' field must be of type 'str'"
        self._returnedmessage = value


class Metaclass_GetSetNav(type):
    """Metaclass of service 'GetSetNav'."""

    _TYPE_SUPPORT = None

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('dave_interfaces')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'dave_interfaces.srv.GetSetNav')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._TYPE_SUPPORT = module.type_support_srv__srv__get_set_nav

            from dave_interfaces.srv import _get_set_nav
            if _get_set_nav.Metaclass_GetSetNav_Request._TYPE_SUPPORT is None:
                _get_set_nav.Metaclass_GetSetNav_Request.__import_type_support__()
            if _get_set_nav.Metaclass_GetSetNav_Response._TYPE_SUPPORT is None:
                _get_set_nav.Metaclass_GetSetNav_Response.__import_type_support__()


class GetSetNav(metaclass=Metaclass_GetSetNav):
    from dave_interfaces.srv._get_set_nav import GetSetNav_Request as Request
    from dave_interfaces.srv._get_set_nav import GetSetNav_Response as Response

    def __init__(self):
        raise NotImplementedError('Service classes can not be instantiated')

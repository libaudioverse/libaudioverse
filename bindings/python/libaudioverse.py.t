{%-import 'macros.t' as macros with context-%}
import _lav
import _libaudioverse
import weakref
import collections

{%macro implement_property(enumerant, prop)%}
	@property
	def {{prop['name']}}(self):
		return _lav.object_get_{{prop['type']}}_property(self.handle, _libaudioverse.{{enumerant}})

	@{{prop['name']}}.setter
	def {{prop['name']}}(self, val):
{%if prop['type'] == 'int'%}
		_lav.object_set_int_property(self.handle, _libaudioverse.{{enumerant}}, int(val))
{%elif prop['type'] == 'float' or prop['type'] == 'double'%}
		_lav.object_set_{{prop['type']}}_property(self.handle, _libaudioverse.{{enumerant}}, float(val))
{%elif prop['type'] == 'float3'%}
		arg_tuple = tuple(val)
		if len(arg_tuple) != 3:
			raise  ValueError('Expected a list or list-like object of 3 floats')
		_lav.object_set_float3_property(self.handle, _libaudioverse.{{enumerant}}, *(float(i) for i in arg_tuple))
{%elif prop['type'] == 'float6'%}
		arg_tuple = tuple(val)
		if len(arg_tuple) != 6:
			raise ValueError('Expected a list or list-like object of 6 floats')
		_lav.object_set_float6_property(self.handle, _libaudioverse.{{enumerant}}, *(float(i) for i in arg_tuple))
{%endif%}
{%endmacro%}

#initialize libaudioverse.  This is per-app and implies no context settings, etc.
_lav.initialize_library()

#This dict maps type codes to classes so that we can revive objects from handles, etc.
_types_to_classes = dict()

#Sometimes, it is possible for the C library to give us a handle without it being implicitly associated with us constructing a type.  In these cases, we have to do something.
#When possible, we want to return an object that already exists.
#otherwise, we need to use some magic to make a new object.
_handles_to_objects = weakref.WeakValueDictionary()

def _wrap(handle):
	"""For private use only. Do not touch."""
	val = _handles_to_objects.get(handle, None)
	if val is not None:
		return val
	val = GenericObject(handle) #the GenericObject simply wraps, doesn't actually construct.
	#this is magic that does work and is apparently used by pickle.
	val.__class__ = _types_to_classes[_lav.object_get_type(handle)]
	return val

#build and register all the error classes.
class GenericError(object):
	"""Base for all libaudioverse errors."""
	pass
{%for error_name, friendly_name in friendly_errors.iteritems()%}
class {{friendly_name}}(GenericError):
	pass
_lav.bindings_register_exception(_libaudioverse.{{error_name}}, {{friendly_name}})
{%endfor%}

#A list-like thing that knows how to manipulate parents.
class ParentProxy(collections.MutableSequence):
	"""Manipulate parents for some specific object.
This works exactly like a python list, save that concatenation is not allowed.  The elements of this list are tuples: (parent, output).

To link a parent to an output using this object, use  obj.parents[num] = (myparent, output).

Note that these objects are always up to date with their associated libaudioverse object but that iterators to them will become outdated if anything changes the graph."""

	def __init__(self, for_object):
		self.for_object = for_object	

	def __len__(self):
		handle = _lav.object_get_input_count(self.for_object.handle)
		if handle is None or handle == 0:
			return None
		return _wrap(handle)

	def __getitem__(self, val):
		return (_lav.object_get_parent_object(obj.handle), _lav.get_parent_output(obj.handle))

	def __setitem__(self, key, val):
		if len(val) != 2:
			raise TypeError("Expected list of length 2.")
		if not isinstance(val[0], GenericObject):
			raise TypeError("val[0]: is not a Libaudioverse object.")
		_lav.object_set_parent(self.for_object.handle, val[0].handle, val[1])

#This is the class hierarchy.
#GenericObject is at the bottom, and we should never see one; and GenericObject should hold most implementation.
class GenericObject(object):
	"""A Libaudioverse object."""

	def __init__(self, handle):
		self.handle = handle
		_handles_to_objects[handle] = self
{%for enumerant, prop in properties['Lav_OBJTYPE_GENERIC'].iteritems()%}
{{implement_property(enumerant, prop)}}
{%endfor%}

_types_to_classes[_libaudioverse.Lav_OBJTYPE_GENERIC] = GenericObject

{%-for object_name, friendly_name in friendly_objects.iteritems()%}
{%set constructor_arg_names = object_constructor_info[object_name].input_args|map(attribute='name')|list-%}
class {{friendly_name}}(GenericObject):
	def __init__(self{%if constructor_arg_names|length > 0%}, {%endif%}{{constructor_arg_names|join(', ')}}):
		super({{friendly_name}}, self).__init__(_lav.{{object_constructors[object_name]}}({{constructor_arg_names|join(', ')}}))
{%for enumerant, prop in properties.get(object_name, dict()).iteritems()%}
{{implement_property(enumerant, prop)}}

{%endfor%}
_types_to_classes[_libaudioverse.{{object_name}}] = {{friendly_name}}
{%endfor%}


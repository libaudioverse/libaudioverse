{%-import 'macros.t' as macros with context-%}
import _lav
import _libaudioverse
import weakref
import collections
import ctypes

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
	if handle is None or handle == 0:
		return None #handle null pointers.
	val = _handles_to_objects.get(handle, None)
	if val is not None:
		return val
	val = _types_to_classes[_lav.object_get_type(handle)].__new__(cls) #We need to avoid the call to __init__.
	#all classes which wrap libaudioverse objects have only a .handle attribute at this time, and are otherwise uniform; consequently, we can fill it in here.
	val.handle = handle
	return val

#build and register all the error classes.
class GenericError(Exception):
	"""Base for all libaudioverse errors."""
	pass

{%for error_name in constants.iterkeys()|prefix_filter("Lav_ERROR_")|remove_filter("Lav_ERROR_NONE")%}
{%set friendly_name = error_name|strip_prefix("Lav_ERROR_")|lower|underscores_to_camelcase(True)%}
class {{friendly_name}}Error(GenericError):
	pass
_lav.bindings_register_exception(_libaudioverse.{{error_name}}, {{friendly_name}}Error)

{%endfor%}

#A list-like thing that knows how to manipulate inputs.
class ParentProxy(collections.Sequence):
	"""Manipulate inputs for some specific object.
This works exactly like a python list, save that concatenation is not allowed.  The elements are tuples: (parent, output) or, should no parent be set for a slot, None.

To link a parent to an output using this object, use  obj.parents[num] = (myparent, output).
To clear a parent, assign None.

Note that these objects are always up to date with their associated libaudioverse object but that iterators to them will become outdated if anything changes the graph.

Note also that we are not inheriting from MutableSequence because we cannot support __del__ and insert, but that the above advertised functionality still works anyway."""

	def __init__(self, for_object):
		self.for_object = for_object	

	def __len__(self):
		return _lav.object_get_input_count(self.for_object.handle)

	def __getitem__(self, key):
		par, out = _lav.object_get_parent_object(self.for_object.handle, key), _lav.object_get_parent_output(self.for_object.handle, key)
		if par is None:
			return None
		return par, out

	def __setitem__(self, key, val):
		if len(val) != 2 and val is not None:
			raise TypeError("Expected list of length 2 or None.")
		if not isinstance(val[0], GenericObject):
			raise TypeError("val[0]: is not a Libaudioverse object.")
		_lav.object_set_parent(self.for_object.handle, key, val[0].handle if val is not None else None, val[1] if val is not None else 0)

class Device(object):
	"""Represents output, either to an audio card or otherwise.  A device is required by all other Libaudioverse objects.

Don't instantiate this class directly.  Use one of the create_ classmethods."""

	def __init__(self, handle):
		self.handle = handle

	@classmethod
	def create_default_audio_output_device(cls):
		"""Returns a device that represents the default soundcard with whatever Libaudioverse determines are the best settings."""
		return cls(_lav.create_default_audio_output_device())

	@classmethod
	def create_read_device(cls, sr, channels, block_size):
		"""Returns a device with no audio output.  Time does not advance for a read device automatically.  You can get one block of data from it by caling get_block."""
		return cls(_lav.create_read_device(sr, channels, block_size))

	def get_block(self):
		"""Returns a block of data.
Calling this on an audio output device will cause the audio thread to skip ahead a block, so don't do that."""
		length = _lav.device_get_block_size(self.handle)*_lav.device_get_channels(self.handle)
		buff = (ctypes.c_float*length)()
		_lav.device_get_block(self.handle, buff)
		return list(buff)

	@property
	def output_object(self):
		"""The object assigned to this property is the object which will play through the device."""
		return _wrap(_lav.device_get_output_object(self.handle))

	@output_object.setter
	def output_object(self, val):
		if not (isinstance(val, GenericObject) or val is None):
			raise TypeError("Expected subclass of Libaudioverse.GenericObject")
		_lav.device_set_output_object(self.handle, val.handle if val is not None else val)

#This is the class hierarchy.
#GenericObject is at the bottom, and we should never see one; and GenericObject should hold most implementation.
class GenericObject(object):
	"""A Libaudioverse object."""

	def __init__(self, handle):
		self.handle = handle
		_handles_to_objects[handle] = self

{%for enumerant, prop in metadata['Lav_OBJTYPE_GENERIC']['properties'].iteritems()%}
{{implement_property(enumerant, prop)}}

{%endfor%}

	def __del__(self):
		if _lav is None:
			#undocumented python thing: if __del__ is called at process exit, globals of this module are None.
			return
		if getattr(self, 'handle', None) is not None:
			_lav.free(self.handle)
		self.handle = None

	@property
	def parents(self):
		"""Returns a ParentProxy, an object that acts like a list of tuples.  The first item of each tuple is the parent object and the second item is the ooutput to which we are connected."""
		return ParentProxy(self)

_types_to_classes[_libaudioverse.Lav_OBJTYPE_GENERIC] = GenericObject

{%for object_name in constants.iterkeys()|prefix_filter("Lav_OBJTYPE_")|remove_filter("Lav_OBJTYPE_GENERIC")%}
{%set friendly_name = object_name|strip_prefix("Lav_OBJTYPE_")|lower|underscores_to_camelcase(True) + "Object"%}
{%set constructor_name = "Lav_create" + friendly_name%}
{%set constructor_arg_names = functions[constructor_name].input_args|map(attribute='name')|list-%}
class {{friendly_name}}(GenericObject):
	def __init__(self{%if constructor_arg_names|length > 0%}, {%endif%}{{constructor_arg_names|join(', ')}}):
		{{constructor_arg_names[0]}} = {{constructor_arg_names[0]}}.handle
		super({{friendly_name}}, self).__init__(_lav.{{constructor_name|without_lav|camelcase_to_underscores}}({{constructor_arg_names|join(', ')}}))

{%for enumerant, prop in metadata.get(object_name, dict()).get('properties', dict()).iteritems()%}
{{implement_property(enumerant, prop)}}

{%endfor%}
_types_to_classes[_libaudioverse.{{object_name}}] = {{friendly_name}}

{%endfor%}


{%-import 'macros.t' as macros with context-%}
import _lav
import _libaudioverse
import weakref
import collections
import ctypes
import enum

{%macro implement_property(enumerant, prop)%}
	@property
	def {{prop['name']}}(self):
{%if prop['type'] == 'int' and 'value_enum' in prop%}
		val = _lav.object_get_int_property(self.handle, _libaudioverse.{{enumerant}})
		return {{prop['value_enum']|without_lav|underscores_to_camelcase(True)}}(val)
{%elif prop['type'] == 'boolean'%}
		return bool(_lav.object_get_int_property(self.handle, _libaudioverse.{{enumerant}}))
{%elif 'array' not in prop['type']%}
		return _lav.object_get_{{prop['type']}}_property(self.handle, _libaudioverse.{{enumerant}})
{%elif prop['type'] == 'float_array'%}
		retval = []
		for i in xrange(_lav.object_get_float_array_property_length(self.handle, _libaudioverse.{{enumerant}})):
			retval.append(_lav.object_read_float_array_property(self.handle, _libaudioverse.{{enumerant}}, i))
		return tuple(retval)
{%elif prop['type'] == 'int_array'%}
		retval = []
		for i in xrange(_lav.object_get_int_array_property_length(self.handle, _libaudioverse.{{enumerant}})):
			retval.append(_lav.object_read_int_array_property(self.handle, _libaudioverse.{{enumerant}}, i))
		return tuple(retval)
{%endif%}

	@{{prop['name']}}.setter
	def {{prop['name']}}(self, val):
{%if 'value_enum' in prop%}
		if not isinstance(val, {{prop['value_enum']|without_lav|underscores_to_camelcase(True)}}) and isinstance(val, enum.IntEnum):
			raise valueError('Attemptn to use wrong enum to set property. Expected instance of {{prop['value_enum']|without_lav|underscores_to_camelcase(True)}}')
		if isinstance(val, enum.IntEnum):
			val = val.value
{%endif%}
{%if prop['type'] == 'int'%}
		_lav.object_set_int_property(self.handle, _libaudioverse.{{enumerant}}, int(val))
{%elif prop['type'] == 'boolean'%}
		_lav.object_set_int_property(self.handle, _libaudioverse.{{enumerant}}, int(bool(val)))
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
{%elif prop['type'] == 'float_array'%}
		if not isinstance(val, collections.Sized):
			raise ValueError('expected an iterable with known size')
		_lav.object_replace_float_array_property(self.handle, _libaudioverse.{{enumerant}}, len(val), val)
{%elif prop['type'] == 'int_array'%}
		if not isinstance(val, collections.Sized):
			raise ValueError('expected an iterable with known size')
		_lav.object_replace.Int_array_property(self.handle, _libaudioverse.{{enumerant}}, len(val), val)
{%endif%}
{%endmacro%}

{%macro implement_callback(name, index)%}
	@property
	def {{name}}_callback(self):
		cb = self._callbacks.get({{index}}, None)
		if cb is None:
			return
		return (cb.callback, cb.extra_arguments)

	@{{name}}_callback.setter
	def {{name}}_callback(self, val):
		global _global_callbacks
		val_tuple = tuple(val) if isinstance(val, collections.Iterable) else (val, )
		if len(val_tuple) == 1:
			val_tuple = (val, ())
		cb, extra_args = val_tuple
		callback_obj = _EventCallbackWrapper(self, {{index}}, cb, extra_args)
		self._callbacks[{{index}}] = callback_obj
		_global_callbacks[self.handle].add(callback_obj)
{%endmacro%}

#initialize libaudioverse.  This is per-app and implies no context settings, etc.
_lav.initialize_library()

#this makes sure that callback objects do not die.
_global_callbacks = collections.defaultdict(set)

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
class InputProxy(collections.Sequence):
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
		par, out = self.for_object._get_input(key)
		if par is None:
			return None
		return par, out

	def __setitem__(self, key, val):
		if len(val) != 2 and val is not None:
			raise TypeError("Expected list of length 2 or None.")
		if not isinstance(val[0], GenericObject):
			raise TypeError("val[0]: is not a Libaudioverse object.")
		self.for_object._set_input(key, val[0] if val is not None else None, val[1] if val is not None else 0)

class _EventCallbackWrapper(object):
	"""Wraps callbacks into something sane.  Do not use externally."""

	def __init__(self, for_object, slot, callback, additional_args):
		self.obj_weakref = weakref.ref(for_object)
		self.additional_arguments = additional_args
		self.slot = slot
		self.callback = callback
		self.fptr = _libaudioverse.LavEventCallback(self)
		_lav.object_set_callback(for_object.handle, slot, self.fptr, None)

	def __call__(self, obj, userdata):
		actual_object = self.obj_weakref()
		if actual_object is None:
			return
		self.callback(actual_object, *self.additional_arguments)

class DeviceInfo(object):
	"""Represents info on a audio device."""

	def __init__(self, latency, channels, name, index):
		self.latency = latency
		self.channels = channels
		self.name = name
		self.index = index

def enumerate_devices():
	"""Returns a list of DeviceInfo representing the devices on the system.

The position in the list is the needed device index for Simulation.__iniit__."""
	max_index = _lav.device_get_count()
	infos = []
	for i in xrange(max_index):
		info = DeviceInfo(index = i,
		latency = _lav.device_get_latency(i),
		channels = _lav.device_get_channels(i),
		name = _lav.device_get_name(i))
		infos.append(info)
	return infos

class Simulation(object):
	"""Represents a running simulation.  All libaudioverse objects must be passed a simulation at creation time and cannot migrate between them.  Furthermore, it is an error to try to connect objects from different simulations."""

	def __init__(self, sample_rate = 44100, block_size = 1024, mix_ahead = 1, channels = 2, device_index = None):
		"""Create a simulation.

See enumerate_devices for the possible values of device_index and other output information.

There are two ways to initialize a device.

If device_index is None, sample_rate, buffer_size, and channels are used to give a simulation that doesn't actually output.  In this case, use the get_block method yourself to retrieve blocks of 32-bit floating point audio data.

Alternatively, if device_index is an integer, a device is created which feeds the specified output.  In this case, sample_rate, block_size, and mix_ahead are respected; channels is determined by the device in question.

One special value is not included in get_device_infos; this is -1.  -1 is the default system audio device plus the functionality required to follow the default if the user changes it, i.e. by unplugging headphones.  In this case, the returned device is always 2 channels."""
		if device_index is not None:
			handle = _lav.create_simulation_for_device(device_index, sample_rate, block_size, mix_ahead)
		else:
			handle = _lav.create_read_simulation(sample_rate, channels, block_size)
		self.handle = handle
		self._output_object = None

	def get_block(self):
		"""Returns a block of data.
Calling this on an audio output device will cause the audio thread to skip ahead a block, so don't do that."""
		length = _lav.simulation_get_block_size(self.handle)*_lav.simulation_get_channels(self.handle)
		buff = (ctypes.c_float*length)()
		#circumvent automatic conversion of iterables.
		buff_ptr = ctypes.POINTER(ctypes.c_float)()
		buff_ptr.contents = buff
		_lav.simulation_get_block(self.handle, buff_ptr)
		return list(buff)

	@property
	def output_object(self):
		"""The object assigned to this property is the object which will play through the device."""
		return self._output_object

	@output_object.setter
	def output_object(self, val):
		if not (isinstance(val, GenericObject) or val is None):
			raise TypeError("Expected subclass of Libaudioverse.GenericObject")
		_lav.simulation_set_output_object(self.handle, val.handle if val is not None else val)
		self._output_object = val

#These are the enums which are needed publicly, i.e. distance model, etc.
{%for name in important_enums%}
{%set constants = constants_by_enum[name]%}
{%set constants_prefix = common_prefix(constants.keys())%}
class {{name|without_lav|underscores_to_camelcase(True)}}(enum.IntEnum):
{%for i, j in constants.iteritems()%}
	{{i|strip_prefix(constants_prefix)|underscores_to_camelcase}} = {{j}}
{%endfor%}
{%endfor%}

#This is the class hierarchy.
#GenericObject is at the bottom, and we should never see one; and GenericObject should hold most implementation.
class GenericObject(object):
	"""A Libaudioverse object."""

	def __init__(self, handle, simulation):
		self.handle = handle
		self.simulation = simulation
		self._inputs = [(None, 0)]*_lav.object_get_input_count(handle)
		self._callbacks = dict()

{%for enumerant, prop in metadata['Lav_OBJTYPE_GENERIC']['properties'].iteritems()%}
{{implement_property(enumerant, prop)}}
{%endfor%}
{%for enumerant, info in metadata['Lav_OBJTYPE_GENERIC'].get('callbacks', dict()).iteritems()%}
{{implement_callback(info['name'], "_libaudioverse." + enumerant)}}
{%endfor%}

	def __del__(self):
		if _lav is None:
			#undocumented python thing: if __del__ is called at process exit, globals of this module are None.
			return
		if getattr(self, 'handle', None) is not None:
			_lav.free(self.handle)
		self.handle = None

	@property
	def inputs(self):
		"""Returns an InputProxy, an object that acts like a list of tuples.  The first item of each tuple is the parent object and the second item is the output to which we are connected."""
		self._check_input_resize()
		return InputProxy(self)

	def _check_input_resize(self):
		new_input_count = _lav.object_get_input_count(self.handle)
		new_input_list = self._inputs
		if new_input_count < len(self._inputs):
			new_input_list = self._inputs[0:new_parents_count]
		elif new_input_count > len(self._inputs):
			additional_inputs = [(None, 0)]*(new_input_count-len(self._inputs))
			new_input_list = new_input_list + additional_inputs
		self._inputs = new_input_list

	def _get_input(self, key):
		return self._inputs[key]

	def _set_input(self, key, obj, inp):
		if obj is None:
			_lav.object_set_input(self.handle, key, None, 0)
			self._inputs[key] = (None, 0)
		else:
			_lav.object_set_input(self.handle, key, obj.handle, inp)
			self._inputs[key] = (obj, inp)

	@property
	def output_count(self):
		"""Get the number of outputs that this object has."""
		return _lav.object_get_output_count(self.handle)

	def reset(self):
		_lav.object_reset(self)

{%for object_name in constants.iterkeys()|prefix_filter("Lav_OBJTYPE_")|remove_filter("Lav_OBJTYPE_GENERIC")%}
{%set friendly_name = object_name|strip_prefix("Lav_OBJTYPE_")|lower|underscores_to_camelcase(True) + "Object"%}
{%set constructor_name = "Lav_create" + friendly_name%}
{%set constructor_arg_names = functions[constructor_name].input_args|map(attribute='name')|map('camelcase_to_underscores')|list-%}
class {{friendly_name}}(GenericObject):
	def __init__(self{%if constructor_arg_names|length > 0%}, {%endif%}{{constructor_arg_names|join(', ')}}):
		super({{friendly_name}}, self).__init__(_lav.{{constructor_name|without_lav|camelcase_to_underscores}}({{constructor_arg_names|join(', ')}}), {{constructor_arg_names[0]}})

{%for enumerant, prop in metadata.get(object_name, dict()).get('properties', dict()).iteritems()%}
{{implement_property(enumerant, prop)}}

{%endfor%}
{%for enumerant, info in metadata.get(object_name, dict()).get('callbacks', dict()).iteritems()%}
{{implement_callback(info['name'], "_libaudioverse." + enumerant)}}

{%endfor%}
{%endfor%}


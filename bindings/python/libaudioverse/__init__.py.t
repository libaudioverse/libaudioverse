{%-import 'macros.t' as macros with context-%}
import _lav
import _libaudioverse
import weakref
import collections
import ctypes
import enum

def find_datafiles():
	import glob
	import platform
	import os.path
	if platform.system() != 'Windows':
		return []
	dlls = glob.glob(os.path.join(__path__[0], '*.dll'))
	return [('libaudioverse', dlls)]


#this makes sure that callback objects do not die.
_global_events= collections.defaultdict(set)

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

#logging infrastructure
_logging_callback = None
_logging_callback_ctypes = None

def set_logging_callback(callback):
	"""Callback must be a function taking 3 arguments: level, message, and is_last.  is_last is set to 1 on the last logging message to be seen, typically found at Libaudioverse shutdown.

use None to clear."""
	global _logging_callback, _logging_callback_ctypes
	callback_c = _libaudioverse.LavLoggingCallback(callback)
	_lav.set_logging_callback(callback_c)
	_logging_callback = callback
	_logging_callback_ctypes = callback_c

def get_logging_callback():
	"""Returns the logging callback."""
	return _logging_callback

def set_logging_level(level):
	"""Set the logging level."""
	_lav.set_logging_level(level)

def get_logging_level():
	"""Get the logging level."""
	return _lav.get_logging_level()

#library initialization and termination.

_initialized = False
def initialize():
	global _initialized
	_lav.initialize()
	_initialized = True

def shutdown():
	global _initialized
	_initialized = False
	_lav.shutdown()

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
		input = self.for_object._get_input(key)
		if input is None:
			return None
		return input

	def __setitem__(self, key, val):
		if val is None:
			self.for_object._set_input(key, None, 0)
			return
		if len(val) != 2:
			raise TypeError("Expected list of length 2 or None.")
		if not isinstance(val[0], GenericObject):
			raise TypeError("val[0]: is not a Libaudioverse object.")
		self.for_object._set_input(key, val[0], val[1])

class _EventCallbackWrapper(object):
	"""Wraps events into something sane.  Do not use externally."""

	def __init__(self, for_object, slot, callback, additional_args):
		self.obj_weakref = weakref.ref(for_object)
		self.additional_arguments = additional_args
		self.slot = slot
		self.callback = callback
		self.fptr = _libaudioverse.LavEventCallback(self)
		_lav.object_set_event(for_object.handle, slot, self.fptr, None)

	def __call__(self, obj, userdata):
		actual_object = self.obj_weakref()
		if actual_object is None:
			return
		self.callback(actual_object, *self.additional_arguments)

class _CallbackWrapper(object):

	def __init__(self, obj, cb, additional_args, additional_kwargs):
		self.additional_args = additional_args
		self.additional_kwargs = additional_kwargs
		self.cb = cb
		self.obj_weakref = weakref.ref(obj)

	def __call__(self, *args):
		needed_args = (self.obj_weakref(),)+args[1:-1] #be sure to eliminate userdata, which is always the last argument.
		return self.cb(*needed_args, **self.additional_kwargs)

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

	def __init__(self, sample_rate = 44100, block_size = 1024, mix_ahead = 2, channels = 2, device_index = -1):
		"""Create a simulation.

See enumerate_devices for the possible values of device_index and other output information.

There are two ways to initialize a device.

If device_index is None, sample_rate and buffer_size are used to give a simulation that doesn't actually output.  In this case, use the get_block method yourself to retrieve blocks of 32-bit floating point audio data.

If device_index is an integer, a device is created which feeds the specified output.  In this case, sample_rate, block_size, channels, and mix_ahead are respected.

One special value is not included in get_device_infos; this is -1.  -1 is the default system audio device plus the functionality required to follow the default if the user changes it, i.e. by unplugging headphones.  Backends will handle mixing to this device appropriately, though getting it's properties is not possible.

See the manual for specifics on how output objects work.  A brief summary is given here: if the output object has 1, 2, 6, or 8 outputs, it is mixed to the output channel count using internal mixing matrices.  Otherwise, the first n outputs are taken as the channels to be outputted and mapped to the channels of the output device.."""
		if device_index is not None:
			handle = _lav.create_simulation_for_device(device_index, channels, sample_rate, block_size, mix_ahead)
		else:
			handle = _lav.create_read_simulation(sample_rate, block_size)
		self.handle = handle
		self._output_object = None

	def get_block(self, channels, may_apply_mixing_matrix = True):
		"""Returns a block of data.
Calling this on an audio output device will cause the audio thread to skip ahead a block, so don't do that."""
		length = _lav.simulation_get_block_size(self.handle)*channels
		buff = (ctypes.c_float*length)()
		#circumvent automatic conversion of iterables.
		buff_ptr = ctypes.POINTER(ctypes.c_float)()
		buff_ptr.contents = buff
		_lav.simulation_get_block(self.handle, channels, may_apply_mixing_matrix, buff_ptr)
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
	{{i|strip_prefix(constants_prefix)|lower}} = {{j}}
{%endfor%}
{%endfor%}

#This is the class hierarchy.
#GenericObject is at the bottom, and we should never see one; and GenericObject should hold most implementation.
class GenericObject(object):
	"""A Libaudioverse object."""

	def __init__(self, handle, simulation):
		self.handle = handle
		self.simulation = simulation
		self._inputs = [None]*_lav.object_get_input_count(handle)
		self._events= dict()
		self._callbacks = dict()
		self._properties = dict()
		property_count = _lav.object_get_property_count(self.handle)
		property_array = _lav.object_get_property_indices(self.handle)
		for i in xrange(property_count):
			name = _lav.object_get_property_name(self.handle, property_array[i])
			self._properties[name] = property_array[i]

{%for enumerant, prop in metadata['objects']['Lav_OBJTYPE_GENERIC']['properties'].iteritems()%}
{{macros.implement_property(enumerant, prop)}}
{%endfor%}
{%for enumerant, info in metadata['objects']['Lav_OBJTYPE_GENERIC'].get('events', dict()).iteritems()%}
{{macros.implement_event(info['name'], "_libaudioverse." + enumerant)}}
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
			new_input_list = self._inputs[0:new_input_count]
		elif new_input_count > len(self._inputs):
			additional_inputs = [None]*(new_input_count-len(self._inputs))
			new_input_list = new_input_list + additional_inputs
		self._inputs = new_input_list

	def _get_input(self, key):
		if self._inputs[key] is None:
			return None
		obj, slot = self._inputs[key]
		handle = obj.handle
		#We need to check and make sure this is still valid.
		#It's possible our parent(s) resized their outputs and that libaudioverse killed this connection.
		if handle != _lav.object_get_input_object(self.handle, key):
			self._inputs[key] = None
			return None
		return obj, slot

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
{%set friendly_name = object_name|strip_prefix("Lav_OBJTYPE_")|lower|underscores_to_camelcase(True)%}
{%set constructor_name = "Lav_create" + friendly_name + "Object"%}
{%set constructor_arg_names = functions[constructor_name].input_args|map(attribute='name')|map('camelcase_to_underscores')|list-%}
class {{friendly_name}}(GenericObject):
	def __init__(self{%if constructor_arg_names|length > 0%}, {%endif%}{{constructor_arg_names|join(', ')}}):
		super({{friendly_name}}, self).__init__(_lav.{{constructor_name|without_lav|camelcase_to_underscores}}({{constructor_arg_names|join(', ')}}), {{constructor_arg_names[0]}})

{%for enumerant, prop in metadata['objects'].get(object_name, dict()).get('properties', dict()).iteritems()%}
{{macros.implement_property(enumerant, prop)}}

{%endfor%}
{%for enumerant, info in metadata['objects'].get(object_name, dict()).get('events', dict()).iteritems()%}
{{macros.implement_event(info['name'], "_libaudioverse." + enumerant)}}

{%endfor%}

{%for func_name, func_info in metadata['objects'].get(object_name, dict()).get('extra_functions', dict()).iteritems()%}
{%set friendly_func_name = func_info['name']%}
{%set func = functions[func_name]%}
{%set lav_func = func.name|without_lav|camelcase_to_underscores%}
	def {{friendly_func_name}}({{func.input_args|map(attribute='name')|list|join(', ')}}):
		return _lav.{{lav_func}}({{func.input_args|map(attribute='name')|list|join(', ')}})

{%endfor%}

{%for callback_name in metadata['objects'].get(object_name, dict()).get('callbacks', [])%}
{%set libaudioverse_function_name = "_lav."+friendly_name|camelcase_to_underscores+"_object_set_"+callback_name+"_callback"%}
{%set ctypes_name = "_libaudioverse.Lav"+friendly_name+"Object"+callback_name|underscores_to_camelcase(True)+"Callback"%}
	def get_{{callback_name}}(self):
		cb = self._callbacks.get("{{callback_name}}", None)
		if cb is None:
			return None
		else:
			return cb[0]

	def set_{{callback_name}}_callback(self, callback, additional_args = None, additional_kwargs = None):
		if callback is None:
			#delete the key, clear the callback with Libaudioverse.
			{{libaudioverse_function_name}}(self.handle, None, None)
			del self._callbacks[{{callback_name}}]
			return
		if additional_args is None:
			additionnal_args = ()
		if additional_kwargs is None:
			additional_kwargs = dict()
		wrapper = _CallbackWrapper(self, callback, additional_args, additional_kwargs)
		ctypes_callback = {{ctypes_name}}(wrapper)
		{{libaudioverse_function_name}}(self.handle, ctypes_callback, None)
		#if we get here, we hold both objects; we succeeded in setting because no exception was thrown.
		#As this is just for GC and the getter, we don't deal with the overhead of an object, and just use tuples.
		self._callbacks["{{callback_name}}"] = (callback, wrapper, ctypes_callback)
{%endfor%}

{%endfor%}


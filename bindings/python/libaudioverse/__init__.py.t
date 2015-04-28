{%-import 'macros.t' as macros with context-%}
import _lav
import _libaudioverse
import weakref
import collections
import ctypes
import enum
import functools
import threading

def find_datafiles():
	import glob
	import platform
	import os.path
	if platform.system() != 'Windows':
		return []
	dlls = glob.glob(os.path.join(__path__[0], '*.dll'))
	return [('libaudioverse', dlls)]


#Everything below here might need the important enums, namely Lav_OBJECT_TYPES:
{%for name in important_enums%}
{%set constants = constants_by_enum[name]%}
{%set constants_prefix = common_prefix(constants.keys())%}
class {{name|without_lav|underscores_to_camelcase(True)}}(enum.IntEnum):
{%for i, j in constants.iteritems()%}
	{{i|strip_prefix(constants_prefix)|lower}} = {{j}}
{%endfor%}
{%endfor%}

#registry of classes to be resurrected if we see a handle and don't already have one.
_types_to_classes = dict()

#Instances that already exist.
_weak_handle_lookup = weakref.WeakValueDictionary()
#Holds a mapping of handles to states.
_object_states = dict()
#This has to be recursive.
#We could be in the middle of an operation that causes resurrection and/or initialization.
#Then the gc collects a _HandleBox, a refcount goes to 0, and we see _handle_destroyed in the same thread.
_object_states_lock = threading.RLock()

#magically resurrect an object from a handle.
def _resurrect(handle):
	obj = _weak_handle_lookup.get(handle, None)
	if obj is None:
		cls = _types_to_classes[ObjectTypes(_lav.handle_get_type(handle))]
		obj = cls.__new__(cls)
		obj.init_with_handle(handle)
	_weak_handle_lookup[handle] = obj
	return obj

#This is the callback for handle destruction.
#This can only be called after both sides have no more references to the object in question.
def _handle_destroyed(handle):
	with _object_states_lock:
		if handle in _object_states:
			del _object_states[handle]

_handle_destroyed_callback=_libaudioverse.LavHandleDestroyedCallback(_handle_destroyed)
_libaudioverse.Lav_setHandleDestroyedCallback(_handle_destroyed_callback)

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

class _EventCallbackWrapper(object):
	"""Wraps events into something sane.  Do not use externally."""

	def __init__(self, for_node, slot, callback, additional_args):
		#We have to hold onto the int representation.
		self.node_handle = for_node.handle.handle
		self.additional_arguments = additional_args
		self.slot = slot
		self.callback = callback
		self.fptr = _libaudioverse.LavEventCallback(self)
		_lav.node_set_event(for_node.handle, slot, self.fptr, None)

	def __call__(self, node, userdata):
		#Throw it in a _HandleBox, and then resurrect.
		#This is safe because we're in a callback and nodes cannot be deleted from callbacks.
		actual_node= _resurrect(_lav._HandleBox(self.node_handle))
		self.callback(actual_node, *self.additional_arguments)

class _CallbackWrapper(object):

	def __init__(self, node, cb, additional_args, additional_kwargs):
		self.additional_args = additional_args
		self.additional_kwargs = additional_kwargs
		self.cb = cb
		self.node_handle = node.handle.handle

	def __call__(self, *args):
		needed_args = (_resurrect(_lav._HandleBox(self.node_handle)), )+args[1:-1] #be sure to eliminate userdata, which is always the last argument.
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

@functools.total_ordering
class _HandleComparer(object):

	def __eq__(self, other):
		if not isinstance(other, _HandleComparer): return False
		return self.handle == other.handle

	def __lt__(self, other):
		#Things that aren't subclasses are less than us.
		if not isinstance(other, _HandleComparer): returnTrue
		return self.handle < other.handle

	def __hash__(self):
		return self.handle.__hash__()

class Simulation(_HandleComparer):
	"""Represents a running simulation.  All libaudioverse nodes must be passed a simulation at creation time and cannot migrate between them.  Furthermore, it is an error to try to connect objects from different simulations.

Instances of this class are context managers.  Using the with statement on an instance of this class invoke's Libaudioverse's atomic block support."""


	def __init__(self, sample_rate = 44100, block_size = 1024, mix_ahead = 2, channels = 2, device_index = -1):
		"""Create a simulation.

See enumerate_devices for the possible values of device_index and other output information.

There are two ways to initialize a device.

If device_index is None, sample_rate and buffer_size are used to give a simulation that doesn't actually output.  In this case, use the get_block method yourself to retrieve blocks of 32-bit floating point audio data.

If device_index is an integer, a device is created which feeds the specified output.  In this case, sample_rate, block_size, channels, and mix_ahead are respected.

One special value is not included in get_device_infos; this is -1.  -1 is the default system audio device plus the functionality required to follow the default if the user changes it, i.e. by unplugging headphones.  Backends will handle mixing to this device appropriately, though getting it's properties is not possible.

See the manual for specifics on how output objects work.  A brief summary is given here: if the output object has 1, 2, 6, or 8 outputs, it is mixed to the output channel count using internal mixing matrices.  Otherwise, the first n outputs are taken as the channels to be outputted and mapped to the channels of the output device."""
		if device_index is not None:
			handle = _lav.create_simulation_for_device(device_index, channels, sample_rate, block_size, mix_ahead)
		else:
			handle = _lav.create_read_simulation(sample_rate, block_size)
		self.init_with_handle(handle)
		_weak_handle_lookup[self.handle] = self

	def init_with_handle(self, handle):
		with _object_states_lock:
			if handle.handle not in _object_states:
				_object_states[handle.handle] = dict()
				_object_states[handle.handle]['lock'] = threading.Lock()
				_object_states[handle.handle]['inputs'] = set()
				_object_states[handle.handle]['block_callback'] = None
			self._state = _object_states[handle.handle]
			self.handle = handle
			self._lock = self._state['lock']

	def get_block(self, channels, may_apply_mixing_matrix = True):
		"""Returns a block of data.
Calling this on an audio output device will cause the audio thread to skip ahead a block, so don't do that."""
		with self._lock:
			length = _lav.simulation_get_block_size(self.handle)*channels
			buff = (ctypes.c_float*length)()
			#circumvent automatic conversion of iterables.
			buff_ptr = ctypes.POINTER(ctypes.c_float)()
			buff_ptr.contents = buff
			_lav.simulation_get_block(self.handle, channels, may_apply_mixing_matrix, buff_ptr)
			return list(buff)

	#context manager support.
	def __enter__(self):
		_lav.simulation_begin_atomic_block(self.handle)

	def __exit__(self, type, value, traceback):
		_lav.simulation_end_atomic_block(self.handle)

	def set_block_callback(self, callback, additional_args=None, additional_kwargs=None):
		with self._lock:
			if callback is not None:
				wrapper = _CallbackWrapper(self, callback, additional_args if additional_args is not None else (), additional_kwargs if additional_kwargs is not None else dict())
				ctypes_callback=_libaudioverse.LavBlockCallback(wrapper)
				_lav.simulation_set_block_callback(self, ctypes_callback, None)
				self._state['block_callback'] = (callback, wrapper, ctypes_callback)
			else:
				_lav.simulation_set_block_callback(self, None)
				self._state['block_callback'] = None

	def get_block_callback(self):
		with self._lock:
			return self._state['block_callback'][0]

_types_to_classes[ObjectTypes.simulation] = Simulation

#Buffer objects.
class Buffer(_HandleComparer):
	"""An audio buffer.

Use load_from_file to read a file or load_from_array to load an iterable."""

	def __init__(self, simulation):
		handle=_lav.create_buffer(simulation)
		self.init_with_handle(handle)
		_weak_handle_lookup[self.handle] = self

	def init_with_handle(self, handle):
		with _object_states_lock:
			if handle.handle not in _object_states:
				_object_states[handle.handle] = dict()
				_object_states[handle.handle]['lock'] = threading.Lock()
				_object_states[handle.handle]['simulation'] = _resurrect(_lav.buffer_get_simulation(handle))
			self._state=_object_states[handle.handle]
			self._lock = self._state['lock']
			self.handle = handle

	def load_from_file(self, path):
		_lav.buffer_load_from_file(self, path)

	def load_from_array(sr, channels, frames, data):
		_lav.buffer_load_from_array(sr, channels, frames, data)

_types_to_classes[ObjectTypes.buffer] = Buffer

class PropertyInfo(object):
	"""Gives info about a property.

type: Type of the property as libaudioverse.PropertyTypes.

name: Name of the property.

range:Range of the property, if applicable.  Otherwise none.

has_dynamic_range: True if the property has a dynamic range (i.e. file node's position property), otherwise False.

Note that this only communicates info.  If changes happen after you requested this instance, they will not be reflected here.
"""

	def __init__(self, name, type, range, has_dynamic_range):
		self.name = name
		self.type = type
		self.range = range
		self.has_dynamic_range = has_dynamic_range

#the following classes implement properties:

class LibaudioverseProperty(object):
	"""Proxy to Libaudioverse properties."""

	def __init__(self, node, slot, getter, setter):
		self._node = node
		self._slot = slot
		self._getter=getter
		self._setter = setter

	@property
	def value(self):
		return self._getter(self._node, self._slot)

	@value.setter
	def set_value(self, val):
		return self._setter(self._node, self._slot, val)

class IntProperty(LibaudioverseProperty):
	"""Proxy to an integer or enumeration property."""

	def __init__(self, node, slot, enum = None):
		super(IntProperty, self).__init__(node = node, slot = slot, getter = None, setter = None)
		self.enum = enum

	@property
	def value(self):
		v = _lav.node_get_int_property(self._node, self._slot)
		if self.enum:
			v = self.enum(v)
		return v

	@value.setter
	def set_value(self, v):
		if isinstance(val, enum.IntEnum):
			if not isintstance(v, self.enum):
				raise valueError('Attemptn to use wrong enum to set property. Expected instance of {}'.format(self.enum.__class__))
			val = val.value
		_lav.node_set_int_property(self._node, self._slot, val)

class AutomatedProperty(LibaudioverseProperty):
	"""A property that supports automation and node connection."""
	pass

class FloatProperty(AutomatedProperty):
	"""Proxy to a float property."""

	def __init__(self, node, slot):
		super(FloatProperty, self).__init__(node = node, slot = slot, getter = _lav.node_get_float_property, setter = _lav.node_set_float_property)

class DoubleProperty(LibaudioverseProperty):
	"""Proxy to a double property."""

	def __init__(self, node, slot):
		super(DoubleProperty, self).__init__(node = node, slot = slot, getter = _lav.node_get_double_property, settre = _lav.node_set_double_property)

class StringProperty(LibaudioverseProperty):
	"""Proxy to a string property."""

	def __init__(self, node, slot):
		super(StringProperty, self).__init__(node = node, slot = slot, getter = _lav.node_get_string_property, setter = _lav.node_set_string_property)

class BufferProperty(LibaudioverseProperty):
	"""Proxy to a buffer property."""
	def __init__(self, node, slot):
		#no getter and setter. This is custom.
		self._node = node
		self._slot = slot

	@property
	def value(self):
		return _resurrect(_lav.node_get_buffer_property(self._node, self._slot))

	@value.setter
	def setValue(self, val):
		if val is None or isinstance(val, Buffer):
			_lav.node_set_buffer_property(self._node, self._slot, val)
		else:
			raise ValueError("Expected a Buffer or None.")

class VectorProperty(LibaudioverseProperty):
	def __init__(self, node, slot, getter, setter, length_limit):
		super(VectorProperty, self).__init__(node = node, slot = slot, getter = getter, setter =setter)
		self._length = length

	#Override setter:
	@LibaudioverseProperty.value.setter
	def set_value(self, val):
		if not isinstance(val, collections.sized):
			raise ValueError("Expected a collections.sized subclass")
		if len(val) != self._length:
			raise ValueError("Expected a {}-element list".format(self._length))
		self.setter(self._node, self._slot, *val)

class Float3Properety(VectorProperty):
	def __init__(self, node, slot):
		super(Float3Property, self).__init__(node, slot, getter =_lav.node_get_float3_property, setter = _lav.node_set_float3_property, length = 3)

class Float6Property(VectorProperty):
	def __init__(self, node, slot):
		super(Float6Property, self).__init__(node = node, slot = slot, gettter =_lav.node_get_float6_property, setter =_lav.node_set_float6_property, length = 6)

#Array properties.
#This is a base class because we have 2, but they have to lock their parent node.
class ArrayProperty(LibaudioverseProperty):

	def __init__(self, node, slot, reader, replacer, length):
		self._node = node
		self._slot = slot
		self._reader=reader
		self._replacer=replacer
		self._length = length

	@property
	def value(self):
		with self._node._lock:
			length = self._length(self._node, self._slot)
			accum = [None]*length
			for i in xrange(length):
				accum[i] = self._reader(self._node, self._slot, i)
		return tuple(accum)

	@value.setter
	def set_value(self, val):
		self._replacer(self._node, self._slot, len(val), *val)

class IntArrayProperty(ArrayProperty):
	def __init__(self, node, slot):
		super(IntArrayProperty, self).__init__(node = node, slot = slot, reader = _lav.node_read_int_array_property,
			writer =_lav.node_write_int_array_property, length = _lav.node_get_int_array_property_length)

class FloatArrayProperty(ArrayProperty):
	def __init__(self, node, slot):
		super(FloatArrayProperty, self).__init__(node = node, slot = slot,
			reader =_lav.node_read_float_array_property,
			writer= _lav.node_write_float_array_property,
			length = _lav.node_get_float_array_property_length
		)

#This is the class hierarchy.
#GenericNode is at the bottom, and we should never see one; and GenericObject should hold most implementation.
class GenericNode(_HandleComparer):
	"""A Libaudioverse object."""

	def __init__(self, handle):
		self.init_with_handle(handle)
		_weak_handle_lookup[self.handle] = self

	def init_with_handle(self, handle):
		with _object_states_lock:
			self.handle = handle
			if handle.handle not in _object_states:
				_object_states[handle.handle] = dict()
				self._state = _object_states[handle.handle]
				self._state['simulation'] = _resurrect(_lav.node_get_simulation(self.handle))
				self._state['events'] = dict()
				self._state['callbacks'] = dict()
				self._state['input_connection_count'] =_lav.node_get_input_connection_count(self)
				self._state['output_connection_count'] = _lav.node_get_output_connection_count(self)
				self._state['inputs'] =set()
				self._state['outputs'] = collections.defaultdict(set)
				self._state['lock'] = threading.Lock()
				self._state['properties'] = dict()
{%for enumerant, prop in metadata['nodes']['Lav_OBJTYPE_GENERIC_NODE']['properties'].iteritems()%}
				self._state['properties']["{{prop['name']}}"] = _libaudioverse.{{enumerant}}
{%endfor%}
			else:
				self._state=_object_states[handle]
			self._lock = self._state['lock']

	def get_property_names(self):
		return self._state['properties'].keys()

	def get_property_info(self, name):
		"""Return info for the property named name."""
		with self._lock:
			if name not in self._state['properties']:
				raise ValueError(name + " is not a property on this instance.")
			index = self._state['properties'][name]
			type = PropertyTypes(_lav.node_get_property_type(self.handle, index))
			range = None
			has_dynamic_range = bool(_lav.node_get_property_has_dynamic_range(self.handle, index))
			if type == PropertyTypes.int:
				range = _lav.node_get_int_property_range(self.handle, index)
			elif type == PropertyTypes.float:
				range = _lav.node_get_float_property_range(self.handle, index)
			elif type == PropertyTypes.double:
				range = _lav.node_get_double_property_range(self.handle, index)
			return PropertyInfo(name = name, type = type, range = range, has_dynamic_range = has_dynamic_range)

	def connect(self, output, node, input):
		with self._lock:
			_lav.node_connect(self, output, node, input)
			self._state['outputs'][output].add((output, weakref.ref))
			node._state['inputs'].add((output, self))

	def connect_simulation(self, output):
		with self._lock:
			_lav.node_connect_simulation(self, output)
			self._state['simulation']._state['inputs'].add(self)

	def disconnect(self, output):
		with self._lock:
			_lav.node_disconnect(self, output)
			for i in self._state['outputs'][output]:
				input, weak =i
				obj=weak.get()
				if obj is not None and (output, self) in obj._state['inputs']:
					obj._state['inputs'].remove((output, self))
			if self in self._state['simulation']._state['inputs']:
				self._state['simulation']._state['inputs'].remove(self)

{%for enumerant, prop in metadata['nodes']['Lav_OBJTYPE_GENERIC_NODE']['properties'].iteritems()%}
{{macros.implement_property(enumerant, prop)}}
{%endfor%}
{%for enumerant, info in metadata['nodes']['Lav_OBJTYPE_GENERIC_NODE'].get('events', dict()).iteritems()%}
{{macros.implement_event(info['name'], "_libaudioverse." + enumerant)}}
{%endfor%}

	def reset(self):
		_lav.node_reset(self)

_types_to_classes[ObjectTypes.generic_node] = GenericNode

{%for node_name in constants.iterkeys()|regexp_filter("Lav_OBJTYPE_\w+_NODE")|remove_filter("Lav_OBJTYPE_GENERIC_NODE")%}
{%set friendly_name = node_name|strip_prefix("Lav_OBJTYPE_")|strip_suffix("_NODE")|lower|underscores_to_camelcase(True)%}
{%set constructor_name = "Lav_create" + friendly_name + "Node"%}
{%set constructor_arg_names = functions[constructor_name].input_args|map(attribute='name')|map('camelcase_to_underscores')|list-%}
{%set property_dict = metadata['nodes'].get(node_name, dict()).get('properties', dict())%}
class {{friendly_name}}Node(GenericNode):
	def __init__(self{%if constructor_arg_names|length > 0%}, {%endif%}{{constructor_arg_names|join(', ')}}):
		super({{friendly_name}}Node, self).__init__(_lav.{{constructor_name|without_lav|camelcase_to_underscores}}({{constructor_arg_names|join(', ')}}))

{%if property_dict | length > 0%}
	def init_with_handle(self, handle):
		with _object_states_lock:
			#our super implementation adds us, so remember if we weren't there.
			should_add_properties = handle.handle not in _object_states
			super({{friendly_name}}Node, self).init_with_handle(handle)
			if should_add_properties:
{%for enumerant, prop in property_dict.iteritems()%}
				self._state['properties']["{{prop['name']}}"] = _libaudioverse.{{enumerant}}
{%endfor%}
{%endif%}

{%for enumerant, prop in property_dict.iteritems()%}
{{macros.implement_property(enumerant, prop)}}

{%endfor%}
{%for enumerant, info in metadata['nodes'].get(node_name, dict()).get('events', dict()).iteritems()%}
{{macros.implement_event(info['name'], "_libaudioverse." + enumerant)}}

{%endfor%}

{%for func_name, func_info in metadata['nodes'].get(node_name, dict()).get('extra_functions', dict()).iteritems()%}
{%set friendly_func_name = func_info['name']%}
{%set func = functions[func_name]%}
{%set lav_func = func.name|without_lav|camelcase_to_underscores%}
	def {{friendly_func_name}}({{func.input_args|map(attribute='name')|list|join(', ')}}):
		return _lav.{{lav_func}}({{func.input_args|map(attribute='name')|list|join(', ')}})

{%endfor%}

{%for callback_name in metadata['nodes'].get(node_name, dict()).get('callbacks', [])%}
{%set libaudioverse_function_name = "_lav."+friendly_name|camelcase_to_underscores+"_node_set_"+callback_name+"_callback"%}
{%set ctypes_name = "_libaudioverse.Lav"+friendly_name+"Node"+callback_name|underscores_to_camelcase(True)+"Callback"%}
	def get_{{callback_name}}(self):
		with self._lock:
			cb = self._state['callbacks'].get("{{callback_name}}", None)
			if cb is None:
				return None
			else:
				return cb[0]

	def set_{{callback_name}}_callback(self, callback, additional_args = None, additional_kwargs = None):
		with self._lock:
			if callback is None:
				#delete the key, clear the callback with Libaudioverse.
				{{libaudioverse_function_name}}(self.handle, None, None)
				del self._state['callbacks'][{{callback_name}}]
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
			self._state['callbacks']["{{callback_name}}"] = (callback, wrapper, ctypes_callback)
{%endfor%}
_types_to_classes[ObjectTypes.{{friendly_name | camelcase_to_underscores}}_node] = {{friendly_name}}Node
{%endfor%}

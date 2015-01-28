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

class _EventCallbackWrapper(object):
	"""Wraps events into something sane.  Do not use externally."""

	def __init__(self, for_node, slot, callback, additional_args):
		self.node_weakref = weakref.ref(for_node)
		self.additional_arguments = additional_args
		self.slot = slot
		self.callback = callback
		self.fptr = _libaudioverse.LavEventCallback(self)
		_lav.node_set_event(for_node.handle, slot, self.fptr, None)

	def __call__(self, node, userdata):
		actual_node= self.node_weakref()
		if actual_node is None:
			return
		self.callback(actual_node, *self.additional_arguments)

class _CallbackWrapper(object):

	def __init__(self, node, cb, additional_args, additional_kwargs):
		self.additional_args = additional_args
		self.additional_kwargs = additional_kwargs
		self.cb = cb
		self.node_weakref = weakref.ref(node)

	def __call__(self, *args):
		needed_args = (self.node_weakref(),)+args[1:-1] #be sure to eliminate userdata, which is always the last argument.
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
		self.handle = handle
		self._output_node= None
		self._connected_nodes= set()

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

	#context manager support.
	def __enter__(self):
		_lav.simulation_begin_atomic_block(self.handle)

	def __exit__(self, type, value, traceback):
		_lav.simulation_end_atomic_block(self.handle)

#These are the enums which are needed publicly, i.e. distance model, etc.
{%for name in important_enums%}
{%set constants = constants_by_enum[name]%}
{%set constants_prefix = common_prefix(constants.keys())%}
class {{name|without_lav|underscores_to_camelcase(True)}}(enum.IntEnum):
{%for i, j in constants.iteritems()%}
	{{i|strip_prefix(constants_prefix)|lower}} = {{j}}
{%endfor%}
{%endfor%}

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

#This is the class hierarchy.
#GenericNode is at the bottom, and we should never see one; and GenericObject should hold most implementation.
class GenericNode(object):
	"""A Libaudioverse object."""

	def __init__(self, handle, simulation):
		self.handle = handle
		self.simulation = simulation
		self._events= dict()
		self._callbacks = dict()
		self.input_connection_count=_lav.node_get_input_connection_count(self)
		self.output_connection_count = _lav.node_get_output_connection_count(self)
		self._input_nodes =dict()
		self._output_nodes = dict()
		for i in xrange(self.input_connection_count):
			self._input_nodes[i] =set()
		for i in xrange(self.output_connection_count):
			self._output_nodes[i]=set()

	def get_property_names(self):
		return self._properties.keys()

	def get_property_info(self, name):
		"""Return info for the property named name."""
		if name not in self._properties:
			raise ValueError(name + " is not a property on this instance.")
		index = self._properties[name]
		type = PropertyTypes(_lav.node_get_property_type(self.handle, index))
		range = None
		has_dynamic_range = bool(_lav.node_get_property_has_dynamic_range(self.handle, index))
		if type == PropertyTypes.int:
			range = _lav.node_get_int_property_range(self.handle, index)
		elif type == PropertyTypes.float:
			range = _lav.node_get_float_property_range()
		elif type == PropertyTypes.double:
			range = _lav.node_get_double_property_range(self.handle, index)
		return PropertyInfo(name = name, type = type, range = range, has_dynamic_range = has_dynamic_range)

	def connect(self, output, node, input):
		_lav.node_connect(self, output, node, input)
		#if that fails, we get an exception. If not, we set this up.
		self._output_nodes[output].add((node, input))
		node._input_nodes[input].add((self, output))

	def connect_simulation(self, output):
		_lav.node_connect_simulation(self, output)
		self.simulation._connected_nodes.add((self, output))

	def disconnect(self, output):
		_lav.node_disconnect(self, output)
		for i in self._output_nodes[output]:
			n, o =i
			n._input_nodes[o].remove((self, output))
		self._output_nodes[output].clear()
		if (self, output) in self.simulation._connected_nodes:
			self.simulation._connected_nodes.remove((self, output))

{%for enumerant, prop in metadata['nodes']['Lav_NODETYPE_GENERIC']['properties'].iteritems()%}
{{macros.implement_property(enumerant, prop)}}
{%endfor%}
{%for enumerant, info in metadata['nodes']['Lav_NODETYPE_GENERIC'].get('events', dict()).iteritems()%}
{{macros.implement_event(info['name'], "_libaudioverse." + enumerant)}}
{%endfor%}

	def __del__(self):
		if _lav is None:
			#undocumented python thing: if __del__ is called at process exit, globals of this module are None.
			return
		if getattr(self, 'handle', None) is not None:
			_lav.free(self.handle)
		self.handle = None

	def reset(self):
		_lav.node_reset(self)

{%for node_name in constants.iterkeys()|prefix_filter("Lav_NODETYPE_")|remove_filter("Lav_NODETYPE_GENERIC")%}
{%set friendly_name = node_name|strip_prefix("Lav_NODETYPE_")|lower|underscores_to_camelcase(True)%}
{%set constructor_name = "Lav_create" + friendly_name + "Node"%}
{%set constructor_arg_names = functions[constructor_name].input_args|map(attribute='name')|map('camelcase_to_underscores')|list-%}
class {{friendly_name}}Node(GenericNode):
	def __init__(self{%if constructor_arg_names|length > 0%}, {%endif%}{{constructor_arg_names|join(', ')}}):
		super({{friendly_name}}Node, self).__init__(_lav.{{constructor_name|without_lav|camelcase_to_underscores}}({{constructor_arg_names|join(', ')}}), {{constructor_arg_names[0]}})

	#This has to be here. Killing this is bad. It is not a no-op.
	def __del__(self):
		super({{friendly_name}}Node, self).__del__()

{%for enumerant, prop in metadata['nodes'].get(node_name, dict()).get('properties', dict()).iteritems()%}
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


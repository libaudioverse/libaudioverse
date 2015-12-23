{%-import 'macros.t' as macros with context-%}
r"""Implements all of the Libaudioverse API.

This is the only module that should be used.  All other modules are private."""
from __future__ import absolute_import
from . import _lav
from . import _libaudioverse
import weakref
import collections
import ctypes
import enum
import functools
import threading
import logging
import six.moves


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
    {%if name in metadata['enumerations']%}"""{{metadata['enumerations'][name]['doc_description']}}"""{%endif%}

{%for i, j in constants.items()%}
    {{i|strip_prefix(constants_prefix)|lower}} = {{j}}
    {%if name in metadata['enumerations']%}"""{{metadata['enumerations'][name]['members'][i]}}"""{%endif%}
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
            #If we gc here and the user is using the simulation as a context manager, then
            #We block until they finish.
            #If they do anything that needs the lock we're holding, lock inversion.
            #This variable holds the dict until after the function ends.
            #Note that this is an integer, not a _HandleBox
            ensure_gc_later = _object_states[handle]
            del _object_states[handle]

_handle_destroyed_callback=_libaudioverse.LavHandleDestroyedCallback(_handle_destroyed)
_libaudioverse.Lav_setHandleDestroyedCallback(_handle_destroyed_callback)

#build and register all the error classes.
class GenericError(Exception):
    r"""Base for all libaudioverse errors."""

    def __init__(self):
        self.file = _lav.error_get_file()
        self.line = _lav.error_get_line()
        self.message = _lav.error_get_message()
        super(GenericError, self).__init__("{} ({}:{})".format(self.message, self.file, self.line))

{%for error_name in constants.keys()|prefix_filter("Lav_ERROR_")|remove_filter("Lav_ERROR_NONE")%}
{%set friendly_name = error_name|strip_prefix("Lav_ERROR_")|lower|underscores_to_camelcase(True)%}
class {{friendly_name}}Error(GenericError):
    r"""{{metadata['enumerations']["Lav_ERRORS"]['members'][error_name]}}"""
    pass
_lav.bindings_register_exception(_libaudioverse.{{error_name}}, {{friendly_name}}Error)

{%endfor%}

#logging infrastructure
def _logging_callback(level, message):
    l=logging.getLogger("libaudioverse")
    if level == LoggingLevels.critical:
        l.critical(message)
    elif level == LoggingLevels.info:
        l.info(message)
    elif level == LoggingLevels.debug:
        l.debug(message)

_logging_callback_ctypes = _libaudioverse.LavLoggingCallback(_logging_callback)
_lav.set_logging_callback(_logging_callback_ctypes)
_lav.set_logging_level(int(LoggingLevels.debug))

#library initialization and termination.

_initialized = False
def initialize():
    r"""Corresponds to Lav_initialize, plus binding specific setup.
    
    Call this before using anything from Libaudioverse."""
    global _initialized
    _lav.initialize()
    _initialized = True

def shutdown():
    r"""Corresponds to Lav_shutdown.
    
    Call this at the end of your application.
    You must call it before the interpreter shuts down. Failure to do so will allow Libaudioverse to call your code during Python's shutdown procedures."""
    global _initialized
    _initialized = False
    _lav.shutdown()

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
    r"""Represents info on a audio device.
    
    Channels is the number of channels for the device.  Name is a unicode string containing a human-readable name.  Index should be used with Simulation.set_output_device.
    
    The caveat from the Libaudioverse manual should be  summarized here:
    channels is not reliable, and your application should default to stereo while providing the user the option to change it."""

    def __init__(self, channels, name, index):
        self.channels = channels
        self.name = name
        self.index = index

def enumerate_devices():
    r"""Returns a list of DeviceInfo representing the devices on the system."""
    max_index = _lav.device_get_count()
    infos = []
    for i in six.moves.range(max_index):
        info = DeviceInfo(index = i,
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
        if not isinstance(other, _HandleComparer): return True
        return self.handle < other.handle

    def __hash__(self):
        #We need to return the handle itself.  The box could be unique.
        return self.handle.handle

class Simulation(_HandleComparer):
    r"""Represents a running simulation.  All libaudioverse nodes must be passed a simulation at creation time and cannot migrate between them.  Furthermore, it is an error to try to connect objects from different simulations.

Instances of this class are context managers.  Using the with statement on an instance of this class invoke's Libaudioverse's atomic block support.

For full details of this class, see the Libaudioverse manual."""

    def __init__(self, sample_rate = 44100, block_size = 1024):
        r"""Creates a simulation."""
        handle = _lav.create_simulation(sample_rate, block_size)
        self.init_with_handle(handle)
        _weak_handle_lookup[self.handle] = self

    def init_with_handle(self, handle):
        with _object_states_lock:
            if handle.handle not in _object_states:
                _object_states[handle.handle] = dict()
                _object_states[handle.handle]['lock'] = threading.Lock()
                _object_states[handle.handle]['block_callback'] = None
            self._state = _object_states[handle.handle]
            self.handle = handle
            self._lock = self._state['lock']

    def set_output_device(self, index, channels=2, min_latency = 0.0, start_latency = 0.1, max_latency = 0.2):
        r"""Sets the output device.
        Use -1 for default system audio. 0 and greater are specific audio devices.
        To enumerate output devices, use enumerate_output_devices."""
        _lav.simulation_set_output_device(self, index, channels, min_latency, start_latency, max_latency)

    def clear_output_device(self):
        r"""Clears the output device, stopping audio and allowing use of get_block again."""
        _lav.simulation_clear_output_device(self)

    def get_block(self, channels, may_apply_mixing_matrix = True):
        r"""Returns a block of data.
        
        This function wraps Lav_getBlock.  Note that calling this on a simulation configured to output audio is an error.
        
        If may_apply_mixing_matrix is True, audio will be automatically converted to the output channel type.  If it is false, channels are either dropped or padded with zeros."""
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
        r"""Lock the simulation."""
        _lav.simulation_lock(self.handle)

    def __exit__(self, type, value, traceback):
        r"""Unlock the simulation."""
        _lav.simulation_unlock(self.handle)

    def set_block_callback(self, callback, additional_args=None, additional_kwargs=None):
        r"""Set a callback to be called every block.
        
        This callback is called as though inside a with block, and takes two positional argguments: the simulation and the simulations' time.
        
        Wraps lav_simulationSetBlockCallback."""
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
        r"""The Python bindings provide the ability to retrieve callback objects.  This function retrieves the set block callback, if any."""
        with self._lock:
            return self._state['block_callback'][0]

    def write_file(self, path, channels, duration, may_apply_mixing_matrix=True):
        r"""Write blocks of data to a file.
        
        This function wraps Lav_simulationWriteFile."""
        _lav.simulation_write_file(self, path, channels, duration, may_apply_mixing_matrix)

    @property
    def threads(self):
        r"""The number of threads the simulation is using for processing.
        
        This wraps Lav_simulationGetThreads and Lav_simulationSetThreads."""
        return _lav.simulation_get_threads(self)
        
    @threads.setter
    def threads(self, value):
        _lav.simulation_set_threads(self, value)

_types_to_classes[ObjectTypes.simulation] = Simulation

#Buffer objects.
class Buffer(_HandleComparer):
    r"""An audio buffer.

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
        r"""Load an audio file.
        
        Wraps Lav_bufferLoadFromFile."""
        _lav.buffer_load_from_file(self, path)

    def load_from_array(self, sr, channels, frames, data):
        r"""Load from an array of interleaved floats.
        
        Wraps Lav_bufferLoadFromArray."""
        _lav.buffer_load_from_array(self, sr, channels, frames, data)

    def get_duration(self):
        r"""Get the duration of the buffer in seconds.
        
        Wraps Lav_bufferGetDuration."""
        return _lav.buffer_get_duration(self)

    def get_length_in_samples(self):
        r"""Returns the length of the buffer in samples.
        
        Wraps Lav_bufferGetLengthInSamples."""
        return _lav.buffer_get_length_in_samples(self)

    def normalize(self):
        r"""Normalizes the buffer.
        
        
        Wraps Lav_bufferNormalize."""
        _lav.buffer_normalize(self)

_types_to_classes[ObjectTypes.buffer] = Buffer

#the following classes implement properties:

class LibaudioverseProperty(object):
    r"""Proxy to Libaudioverse properties.
    
    All properties support resetting and type query."""

    def __init__(self, handle, slot, getter, setter):
        self._handle = handle
        self._slot = slot
        self._getter=getter
        self._setter = setter

    @property
    def value(self):
        return self._getter(self._handle, self._slot)

    @value.setter
    def value(self, val):
        return self._setter(self._handle, self._slot, val)

    def reset(self):
        _lav.node_reset_property(self._handle, self._slot)

    @property
    def type(self):
        """The property's type."""
        return PropertyTypes(_lav.node_get_property_type(self._handle, self._slot))

    def __repr__(self):
        return "<{} {}>".format(self.__class__.__name__, self.value)

class BooleanProperty(LibaudioverseProperty):
    r"""Represents a boolean property.
    
    Note that boolean properties show up as int properties when their type is queried.
    This class adds extra marshalling to make sure that boolean properties show up as booleans on the Python side, as the C API does not distinguish between boolean properties and int properties with range [0, 1]."""
    
    def __init__(self, handle, slot):
        super(BooleanProperty, self).__init__(handle = handle, slot = slot, getter =_lav.node_get_int_property, setter = _lav.node_set_int_property)

    @LibaudioverseProperty.value.getter
    def value(self):
        return bool(self._getter(self._handle, self._slot))

class IntProperty(LibaudioverseProperty):
    r"""Proxy to an integer or enumeration property."""

    def __init__(self, handle, slot, enum = None):
        super(IntProperty, self).__init__(handle = handle, slot = slot, getter = None, setter = None)
        self.enum = enum

    @property
    def value(self):
        v = _lav.node_get_int_property(self._handle, self._slot)
        if self.enum:
            v = self.enum(v)
        return v

    @value.setter
    def value(self, val):
        if isinstance(val, enum.IntEnum):
            if not isinstance(val, self.enum):
                raise ValueError('Attemptn to use wrong enum to set property. Expected instance of {}'.format(self.enum.__class__))
            val = val.value
        _lav.node_set_int_property(self._handle, self._slot, val)

class AutomatedProperty(LibaudioverseProperty):
    r"""A property that supports automation and node connection."""

    def linear_ramp_to_value(self, time, value):
        """Schedule a linear automator.
        
        The property's value will change to the specified value by the specified time, starting at the end of the previous automator
        
        This function wraps Lav_automationLinearRampToValue."""
        _lav.automation_linear_ramp_to_value(self._handle, self._slot, time, value)

    def envelope(self, time, duration, values):
        r"""Run an envelope.
        
        The property's value will stay where it was after the last automator until the specified time is reached, whereupon it will follow the envelope until time+duration.
        
        This function wraps Lav_automationEnvelope."""
        values_length = len(values)
        _lav.automation_envelope(self._handle, self._slot, time, duration, values_length, values)

    def set(self, time, value):
        r"""Sets the property's value to a specific value at a specific time.
        
        Wraps Lav_automationSet."""
        _lav.automation_set(self._handle, self._slot, time, value)

    def cancel_automators(self, time):
        r"""Cancel all automators scheduled to start after time.
        
        Wraps Lav_automationCancelAutomators."""
        _lav.automation_cancel_automators(self._handle, self._slot, time)

class FloatProperty(AutomatedProperty):
    r"""Proxy to a float property."""

    def __init__(self, handle, slot):
        super(FloatProperty, self).__init__(handle = handle, slot = slot, getter = _lav.node_get_float_property, setter = _lav.node_set_float_property)

class DoubleProperty(LibaudioverseProperty):
    r"""Proxy to a double property."""

    def __init__(self, handle, slot):
        super(DoubleProperty, self).__init__(handle = handle, slot = slot, getter = _lav.node_get_double_property, setter = _lav.node_set_double_property)

class StringProperty(LibaudioverseProperty):
    r"""Proxy to a string property."""

    def __init__(self, handle, slot):
        super(StringProperty, self).__init__(handle = handle, slot = slot, getter = _lav.node_get_string_property, setter = _lav.node_set_string_property)

class BufferProperty(LibaudioverseProperty):
    r"""Proxy to a buffer property.
    
    It is safe to set this property to None."""
    
    def __init__(self, handle, slot):
        #no getter and setter. This is custom.
        self._handle = handle
        self._slot = slot

    @property
    def value(self):
        return _resurrect(_lav.node_get_buffer_property(self._handle, self._slot))

    @value.setter
    def value(self, val):
        if val is None or isinstance(val, Buffer):
            _lav.node_set_buffer_property(self._handle, self._slot, val if val is not None else 0)
        else:
            raise ValueError("Expected a Buffer or None.")

class VectorProperty(LibaudioverseProperty):
    r"""class to act as a base for  float3 and float6 properties.
    
    This class knows how to marshal anything that is a collections.sized and will error if length constraints are not met."""

    def __init__(self, handle, slot, getter, setter, length):
        super(VectorProperty, self).__init__(handle = handle, slot = slot, getter = getter, setter =setter)
        self._length = length

    #Override setter:
    @LibaudioverseProperty.value.setter
    def value(self, val):
        if not isinstance(val, collections.Sized):
            raise ValueError("Expected a collections.sized subclass")
        if len(val) != self._length:
            raise ValueError("Expected a {}-element list".format(self._length))
        self._setter(self._handle, self._slot, *val)

class Float3Property(VectorProperty):
    r"""Represents a float3 property."""
    
    def __init__(self, handle, slot):
        super(Float3Property, self).__init__(handle = handle, slot = slot, getter =_lav.node_get_float3_property, setter = _lav.node_set_float3_property, length = 3)

class Float6Property(VectorProperty):
    r"""Represents a float6 property."""
    
    def __init__(self, handle, slot):
        super(Float6Property, self).__init__(handle = handle, slot = slot, getter =_lav.node_get_float6_property, setter =_lav.node_set_float6_property, length = 6)

#Array properties.
#This is a base class because we have 2, but they have to lock their parent node.
class ArrayProperty(LibaudioverseProperty):
    r"""Base class for all array properties."""

    def __init__(self, handle, slot, reader, replacer, length, lock):
        self._handle = handle
        self._slot = slot
        self._reader=reader
        self._replacer=replacer
        self._length = length
        self._lock = lock

    @property
    def value(self):
        r"""The array, as a tuple."""
        with self._lock:
            length = self._length(self._handle, self._slot)
            accum = [None]*length
            for i in six.moves.range(length):
                accum[i] = self._reader(self._handle, self._slot, i)
        return tuple(accum)

    @value.setter
    def value(self, val):
        self._replacer(self._handle, self._slot, len(val), val)

class IntArrayProperty(ArrayProperty):
    r"""Represents an int array property."""
    def __init__(self, handle, slot, lock):
        super(IntArrayProperty, self).__init__(handle = handle, slot = slot, lock = lock, reader = _lav.node_read_int_array_property,
            replacer =_lav.node_replace_int_array_property, length = _lav.node_get_int_array_property_length)

class FloatArrayProperty(ArrayProperty):
    r"""Represents a float array property."""

    def __init__(self, handle, slot, lock):
        super(FloatArrayProperty, self).__init__(handle = handle, slot = slot, lock = lock,
            reader =_lav.node_read_float_array_property,
            replacer = _lav.node_replace_float_array_property,
            length = _lav.node_get_float_array_property_length
        )

#This is the class hierarchy.
#GenericNode is at the bottom, and we should never see one; and GenericObject should hold most implementation.
class GenericNode(_HandleComparer):
    r"""Base class for all Libaudioverse nodes.
    
    All properties and functionality on this class is available to all Libaudioverse nodes without exception."""

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
                self._state['callbacks'] = dict()
                self._state['input_connection_count'] =_lav.node_get_input_connection_count(self)
                self._state['output_connection_count'] = _lav.node_get_output_connection_count(self)
                self._state['lock'] = threading.Lock()
                self._state['properties'] = dict()
                self._state['property_instances'] = dict()
{%for enumerant, prop in metadata['nodes']['Lav_OBJTYPE_GENERIC_NODE']['properties'].items()%}
                self._state['properties']["{{prop['name']}}"] = _libaudioverse.{{enumerant}}
{%endfor%}
            else:
                self._state=_object_states[handle.handle]
            self._lock = self._state['lock']
            self._property_instances = dict()
{%for enumerant, prop in metadata['nodes']['Lav_OBJTYPE_GENERIC_NODE']['properties'].items()%}
{{macros.make_property_instance(enumerant, prop)|indent(12, True)}}
{%endfor%}

    def get_property_names(self):
        r"""Get the names of all properties on this node."""
        return self._state['properties'].keys()

    def connect(self, output, node, input):
        r"""Connect the specified output of this node to the specified input of another node.
        
        Nodes are kept alive if another node's input is connected to one of their outputs.
        So long as some node which this node is connected to is alive, this node will also be alive."""
        _lav.node_connect(self, output, node, input)

    def connect_simulation(self, output):
        r"""Connect the specified output of this node to  this node's simulation.
        
        Nodes which are connected to the simulation are kept alive as long as they are connected to the simulation."""
        _lav.node_connect_simulation(self, output)

    def connect_property(self, output, property):
        r"""Connect an output of this node to an automatable property.
        
        Example: n.connect_property(0, mySineNode.frequency).
        
        As usual, this connection keeps this node alive as long as the destination is also alive."""
        other = property._handle
        slot = property._slot
        _lav.node_connect_property(self, output, other, slot)

    def disconnect(self, output, node = None, input = 0):
        r"""Disconnect from other nodes.
        
        If node is None, all connections involving output are cleared.
        
        if node is not None, then we are disconnecting from a specific node and input combination."""
        if node is None:
            node = 0 #Force this translation.
        _lav.node_disconnect(self, output, node, input)

    def isolate(self):
        r"""Disconnect all outputs."""
        _lav.node_isolate(self)

{%for enumerant, prop in metadata['nodes']['Lav_OBJTYPE_GENERIC_NODE']['properties'].items()%}
{{macros.implement_property(enumerant, prop)}}
{%endfor%}

    def reset(self):
        r"""Perform the node-specific reset operation.
        
        This directly wraps Lav_nodeReset."""
        _lav.node_reset(self)

_types_to_classes[ObjectTypes.generic_node] = GenericNode

{%for node_name in constants.keys()|regexp_filter("Lav_OBJTYPE_\w+_NODE")|remove_filter("Lav_OBJTYPE_GENERIC_NODE")%}
{%set friendly_name = node_name|strip_prefix("Lav_OBJTYPE_")|strip_suffix("_NODE")|lower|underscores_to_camelcase(True)%}
{%set constructor_name = "Lav_create" + friendly_name + "Node"%}
{%set constructor_arg_names = functions[constructor_name].input_args|map(attribute='name')|map('camelcase_to_underscores')| map('strip_suffix', "_handle")| list-%}
{%set property_dict = metadata['nodes'].get(node_name, dict()).get('properties', dict())%}
class {{friendly_name}}Node(GenericNode):
    r"""{{metadata['nodes'][node_name].get('doc_description', "No descriptiona vailable.")}}"""
    
    def __init__(self{%if constructor_arg_names|length > 0%}, {%endif%}{{constructor_arg_names|join(', ')}}):
        super({{friendly_name}}Node, self).__init__(_lav.{{constructor_name|without_lav|camelcase_to_underscores}}({{constructor_arg_names|join(', ')}}))

    def init_with_handle(self, handle):
        with _object_states_lock:
            #our super implementation adds us, so remember if we weren't there.
            should_add_properties = handle.handle not in _object_states
            super({{friendly_name}}Node, self).init_with_handle(handle)
{%if property_dict|length%}
            if should_add_properties:
{%for enumerant, prop in property_dict.items()%}
                self._state['properties']["{{prop['name']}}"] = _libaudioverse.{{enumerant}}
{%endfor%}
{%for enumerant, prop in property_dict.items()%}
{{macros.make_property_instance(enumerant, prop)|indent(12,  True)}}
{%endfor%}
{%endif%}

{%for enumerant, prop in property_dict.items()%}
{{macros.implement_property(enumerant, prop)}}

{%endfor%}

{%for func_name, func_info in metadata['nodes'].get(node_name, dict()).get('extra_functions', dict()).items()%}
{%set friendly_func_name = func_info['name']%}
{%set func = functions[func_name]%}
{%set lav_func = func.name|without_lav|camelcase_to_underscores%}
{%set input_args= func.input_args|map(attribute='name')|map('camelcase_to_underscores')|map('strip_suffix', '_handle')|list|join(', ')%}
    def {{friendly_func_name}}({{input_args}}):
        r"""{{func_info.get('doc_description', "No description available.")}}"""
        return _lav.{{lav_func}}({{input_args}})

{%endfor%}

{%for callback_name, callback_info in metadata['nodes'].get(node_name, dict()).get('callbacks', dict()).items()%}
{%set libaudioverse_function_name = "_lav."+friendly_name|camelcase_to_underscores+"_node_set_"+callback_name+"_callback"%}
{%set ctypes_name = "_libaudioverse.Lav"+friendly_name+"Node"+callback_name|underscores_to_camelcase(True)+"Callback"%}
    def get_{{callback_name}}_callback(self):
        r"""Get the {{callback_name}} callback.
        
        This is a feature of the Python bindings and is not available in the C API.  See the setter for specific documentation on this callback."""
        with self._lock:
            cb = self._state['callbacks'].get("{{callback_name}}", None)
            if cb is None:
                return None
            else:
                return cb[0]

    def set_{{callback_name}}_callback(self, callback, additional_args = None, additional_kwargs = None):
        r"""Set the {{callback_name}} callback.
        
{{callback_info.get("doc_description", "No description available.")}}"""
        with self._lock:
            if callback is None:
                #delete the key, clear the callback with Libaudioverse.
                {{libaudioverse_function_name}}(self.handle, None, None)
                del self._state['callbacks']['{{callback_name}}']
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

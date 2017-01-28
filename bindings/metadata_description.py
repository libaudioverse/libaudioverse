"""This file defines the DSL for describing Libaudioverse objects and functions.

You want to see class dsl.Builder.  Files in metadata/* are executed with the builder helper functions in the global namespace."""

import enum

# Helpful constant for consumers: infinity.
inf = float('inf')

class TypeInfo:
    """Describes a type.  Members:

base: The base type as a string or FunctionInfo.  If FunctionInfo, indirection is at least 1.
indirection: the number of pointer indirections.
quals: The qualifiers at each indirection. Keys are ints.

Whether or not this TypeInfo can represent a typedef depends on context. Objects in this file that use TypeInfo will translate where necessary."""
    def __init__(self, base, indirection, quals = dict()):
        self.base = base
        self.indirection = indirection
        self.quals = quals


class FunctionCategory:
    """Holds information about a function category.

Function categories primarily exist to differentiate sections in the main manual.  Example: node functions, server functions, etc.

name: The name of the category.
doc_name: The heading for the category in documentation.
doc_description: The description of the category in documentation.
functions: the functions in this category, sorted by name.

All functions which are documented and not associated with an object have a category.
"""

    def __init__(self, name, doc_name, doc_description, functions):
        self.name = name
        self.doc_name = doc_name
        self.doc_description = doc_description
        self.functions = functions

class Function:
    """Describes a function.

name: The name of this function.
doc:  The documentation. None if this function is undocumented or used to get/set a callback.
return_type: The return type with typedefs translated.
return_type_pretty: The return type without typedefs translated.
params: A list of parameters, either SimpleParam or ArrayParam.
input_params: All parameters which are inputs.
output_params: All parameters which are outputs.
category: None, or a FunctionCategory instance. Describes what heading this function belongs under in the language-agnostic manual.

Functions don't have a category if they are for getting/setting callbacks, not documented, or specific to a node.
"""

    def __init__(self, return_type, return_type_pretty, name, doc, params, category):
        self.return_type = return_type
        self.return_type_pretty = return_type_pretty
        self.name = name
        self.params = tuple(params)
        self.input_params = tuple([i for i in params if not i.output])
        self.output_params = tuple([i for i in params if i.output])
        self.category = category
        self.doc = doc

class SimpleParam:
    """Describes a single parameter, passed as a single value.

name: The name of the parameter.
output: true if this parameter is an output parameter, otherwise false.
type: The type with typedefs translated.
type_pretty: The type without typedefs translated.
default: The default value of the param, represented as an appropriate Python object. None if there is no default, and 0 for null pointers.
doc: The documentation for the parameter. May be None in any case; warnings are printed in the bindings dsl if this isn't because it's a callback getter/setter or undocumented function.
"""

    _uses = 1 # For part of the bindings DSL. How many parameters does this consume?

    def __init__(self, name, type, type_pretty, doc, output = None):
        self.type = type
        self.type_pretty = type_pretty
        self.name = name
        self.doc = doc
        if output is None:
            output = "destination" in name.lower()
        self.output = output


class ArrayParam:
    """An array parameter.

This is essentially a pair of parameters, the first of which is always a length and the second of which is always a pointer.

params: params[0] is the ,length parameter. params[1] is the pointer.
output: True if this is an output parameter.

When computing documentation, use the second parameter for languages which don't require explicitly passing lengths.  A property is provided here for convenience.
"""

    _uses = 2 # For part of the bindings DSL. How many parameters does this consume?
    def __init__(self, params, output = None):
        self.params = params
        if output is None:
            if self.params[0].output != self.params[1].output:
                raise ValueError("Array parameter with one input and one output parameter")
                output = params[0].output
        self.output = output

    @property
    def doc(self):
        return self.params[1].doc

class Enum:
    """Represents a C enum

name: The name.
doc: The description of the enum if provided, otherwise None.
members: A  dict mapping member names to EnumMember.
"""
    def __init__(self, name, members, doc):
        self.name = name
        self.members = members
        self.doc = doc

class EnumMember:
    """Represents an enum member.

name: The name.
value: The value.
doc: The description of the member, or None if not provided.
"""

    def __init__(self, name, value, doc):
        self.name = name
        self.value = value
        self.doc = doc

ChannelTypes = enum.Enum("ChannelTypes", "constant dynamic constructor")

class ConnectionPoint:
    """Represents an input to or output from a node.  See the following two subclasses.  All of the following properties are specified:

doc: What the input is for.
channel_type: Is this input's channel count constant, dynamic, or set by the constructor?
channels: A number of channels if constant, None if set by the constructor of the node, or a string describing where it comes from.
"""
    def __init__(self, doc, channel_type, channels):
        self.doc = doc
        self.channel_type = channel_type
        self.channels = channels


class Input(ConnectionPoint):
    """Represents an input to a node. See ConnectionPoint docs for details."""
    pass

class Output(ConnectionPoint):
    """Represents an output from a node. See ConnectionPoint docs for details."""
    pass


class Node:
    """Represents a node.

identifier: The Lav_OBJTYPE_XXX constant name as a string.
doc_name: The description for this node, used as the header in the language-agnostic manual.
doc_description: The long-form description of this node.
properties: A list of property instances, sorted in alphabetical order by name.
callbacks: A list of callbacks, sorted in alphabetical order by name.
extra_functions: A list of extra functions, sorted in alphabetical order by the name of the function representing them in C.  Uses instances of Function, annotated with documentation from the node's description.
inputs: A list of input instances in order by index.
outputs: A list of output instances in order by index.
"""
    def __init__(self, identifier, doc_name, doc_description, properties, callbacks, extra_functions, inputs, outputs):
        self.identifier = identifier
        self.doc_name = doc_name
        self.doc_description = doc_description
        self.properties = properties
        self.callbacks = callbacks
        self.extra_functions = extra_functions
        self.inputs = inputs
        self.outputs = outputs

PropertyTypes = enum.Enum("PropertyTypes", "boolean int float double float3 float6 buffer int_array float_array")
RangeTypes = enum.Enum("RangeTypes", "specified dynamic constructor")
DefaultTypes = enum.Enum("DefaultTypes", "specified constructor")
AccessTypes = enum.Enum("AccessTypes", "writable readonly varies")

class Property:
    """Represents a property.

name: The name.
doc: The description.  Always present.
identifier: The name of the C constant representing this property, as a string.
type: A PropertyTypes.
range_type: A RangeTypes.
range: A tuple (min, max) if range is constant. Otherwise a string description for dynamic ranges or None if the range is set by the constructor.
default_type: a DefaultTypes.
default: Either a default value of appropriate type for the property or None in the case of buffer properties, array properties, and ranges set by constructors.  For int properties using associated_enum, a string value, the name of the enum constant in C.
access_type: AccessTypes.readonly if the property can only be read, AccessTypes.writable if the property can be written, or AccessTypes.varies if readonly status depends on complex conditions.
access_type_notes: On properties that use AccessTypes.varies, describes how this varies.
array_length_range: Either a tuple (min, max), a string describing the range, or None if it's from the constructor. Unspecified value for non-array properties.
array_length_range_type: A RangeTypes constant, see docs on range and range_type.  Unspecified value for non-array properties.
associated_enum: The name of the C enum from which this property gets its values, if specified. Otherwise None.  Applies only to int and int array properties.

Note that range tuples may contain infinity, made available in this module as inf. For the array length ranges and any int properties, this means MIN_INT or MAX_INT depending on the sign.
"""
    def __init__(self, name, doc, type, identifier,
        range_type, range, default_type, default,
        access_type, access_type_notes,
        array_length_range, array_length_range_type, associated_enum):
        self.name = name
        self.doc = doc
        self.type = type
        self.identifier = identifier
        self.range_type = range_type
        self.range = range
        self.default_type = default_type
        self.default = default
        self.access_type = access_type
        self.access_type_notes = access_type_notes
        self.array_length_range = array_length_range
        self.array_length_range_type = array_length_range_type
        self.associated_enum = associated_enum

class Callback:
    """Represents a callback.

name: The name of the callback.
doc: The callback's documentation.
setter: The C function which sets this callback.
signature: A FunctionInfo representing the callback's signature. Annotated with parameter docs, etc.
signature_typedef: The name of the C typedef representing this callback.
in_audio_thread: true if the callback is called in the audio thread.
"""
    def __init__(self, name, setter, signature, signature_typedef,
        in_audio_thread, doc):
        self.name = name
        self.setter = setter
        self.in_audio_thread = in_audio_thread
        self.signature = signature
        self.signature_typedef = signature_typedef
        self.doc = doc

class Metadata:
    """Holds all the stuff.

function_categories: A dict mapping category names to FunctionCategory instance.
functions: A dict, maps function names to FunctionInfo.  Includes undocumented functions, callback setters, etc.
nodes: A dict of node instances mapping identifier to node instance.
enums: A dict of enums, including all undocumented ones.
documented_enums: Documented enums.
typedefs: A ordereddict of typedef name to TypeInfo representing it.  These are ordered by their appearance in the source.
"""
    def __init__(self, function_categories, functions, nodes, enums, documented_enums, typedefs):
        self.functions = functions
        self.function_Categories = function_categories
        self.enums = enums
        self.documented_enums = documented_enums
        self.nodes = nodes
        self.typedefs = typedefs

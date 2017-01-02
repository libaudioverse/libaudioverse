"""This file defines the DSL for describing Libaudioverse objects and functions.

You want to see class Builder.  Files in metadata/* are executed with the builder helper functions in the global namespace."""
import enum

class TypeInfo:
    """Describes a type.  Members:

base: The base type as a string.
indirection: the number of pointer indirections.
quals: The qualifiers at each indirection. Keys are ints.

Whether or not this TypeInfo can represent a typedef depends on context. Objects in this file that use TypeInfo will translate where necessary."""
    def __init__(self, base, indirection, quals = dict()):
        self.base = base
        self.indirection = indirection
        self.quals = quals


class FunctionCategory:
    """Holds information about a function category.

name: The name of the category.
doc_name: The heading for the category in documentation.
doc_description: The description of the category in documentation.

All functions which are documented and not associated with an object have a category.
"""

    def __init__(self, name, doc_name, doc_description):
        self.name = name
        self.doc_name = doc_name
        self.doc_description = doc_description

class FunctionInfo:
    """Describes a function.

name: The name of this function.
return_type: The return type with typedefs translated.
return_type_pretty: The return type without typedefs translated.
params: A list of parameters, either SimpleParam or ArrayParam.
input_params: All parameters which are inputs.
output_params: All parameters which are outputs.
category: None, or a FunctionCategory instance. Describes what heading this function belongs under in the language-agnostic manual.
"""

    def __init__(self, return_type, return_type_pretty, name, params, category):
        self.return_type = return_type
        self.return_type_pretty = return_type_pretty
        self.name = name
        self.params = tuple(params)
        self.input_params = tuple([i for i in params if not i.output])
        self.output_params = tuple([i for i in params if i.output])
        self.category = category

class SimpleParam:
    """Describes a single parameter, passed as a single value.

name: The name of the parameter.
output: true if this parameter is an output parameter, otherwise false.
type: The type with typedefs translated.
type_pretty: The type without typedefs translated.
default: The default value of the param, represented as an appropriate Python object. None if there is no default, and 0 for null pointers.
"""

    _uses = 1 # For part of the bindings DSL. How many parameters does this consume?

    def __init__(self, name, type, type_pretty, doc_description, output = None):
        self.type = type
        self.type_pretty = type_pretty
        self.name = name
        self.doc_description = doc_description
        if output is None:
            output = "destination" in name.lower()
        self.output = output


class ArrayParam:
    """An array parameter.

This is essentially a pair of parameters, the first of which is always a length and the second of which is always a pointer.


params: params[0] is the ,length parameter. params[1] is the pointer.
output: True if this is an output parameter.
"""

    _uses = 2 # For part of the bindings DSL. How many parameters does this consume?
    def __init__(self, params, output = None):
        self.params = params
        if output is None:
            if self.params[0].output != self.params[1].output:
                raise ValueError("Array parameter with one input and one output parameter")
                output = params[0].output
        self.output = output

class Enum:
    """Represents a C enum

name: The name.
members: A  dict mapping member names to EnumMember.
"""
    def __init__(self, name, members):
        self.name = name
        self.members = members

class EnumMember:
    """Represents an enum member.

name: The name.
value: The value.
doc: The description of the member.
"""

    def __init__(self, name, value, doc):
        self.name = name
        self.value = value
        self.doc = doc

class Node:
    """Represents a node.

identifier: The Lav_OBJTYPE_XXX constant as a string.
doc_name: The description for this node, used as the header in the language-agnostic manual.
doc_description: The long-form description of this node.
properties: A list of property instances, sorted in alphabetical order by name.
callbacks: A list of callbacks, sorted in alphabetical order by name.
extra_functions: A list of extra functions, sorted in alphabetical order by the name of the function representing them in C.
"""
    def __init__(self, identifier, doc_name, doc_description, properties, callbacks, extra_functions):
        self.identifier = identifier
        self.doc_name = doc_name
        self.doc_description = doc_description
        self.properties = properties
        self.callbacks = callbacks
        self.extra_functions = extra_functions

PropertyTypes = enum.Enum("PropertyTypes", "int float double float3 float6 buffer")
RangeTypes = enum.Enum("RangeTypes", "specified, dynamic, constructor")
DefaultTypes = enum.Enum("DefaultTypes", "specified constructor")
AccessTypes = enum.Enum("AccessTypes", "writable readonly varies")

class Property:
    """Represents a property.

name: The name.
doc_description: The description.
type: A PropertyTypes.
range_type: A RangeTypes.
range: A tuple (min, max) if range is specified. Otherwise a string description.
default_type: a DefaultTypes.
default: Either a default value of appropriate type for the property, None if this is a buffer property, or a string description.
access_type: AccessTypes.readonly if the property can only be read, AccessTypes.writable if the property can be written, or AccessTypes.varies if readonly status depends on complex conditions.
access_type_notes: On properties that use AccessTypes.varies, describes how this varies.
"""
    def __init__(self, name, doc_description, type, 
        range_type, range, default_type, default,
        access_type, access_type_notes):
        self.name = name
        self.doc_description = doc_description
        self.type = type
        self.range_type = range_type
        self.range = range
        self.default_type = default_type
        self.default = default
        self.access_type = access_type
        self.access_type_notes = access_type_notes

class Callback:
    """Represents a callback.

name: The name of the callback.
getter: The C function which gets this callback.
setter: The C function which sets this callback.
in_audio_thread: true if the callback is called in the audio thread.
type: A FunctionInfo describing the callback's signature.
"""
    def __init__(self, name, getter, setter,
        in_audio_thread, type):
        self.name = name
        self.getter = getter
        self.setter = setter
        self.in_audio_thread = in_audio_thread
        self.type = type

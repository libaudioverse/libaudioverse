"""This is the bindings DSL.

An instance of Builder is created by the bindings generator. Then, all .py files in metadata/* are read into strings and executed with the methods of the Builder instance and everything else public this module imports or defines in the global namespace.

As a convenience, this module accepts strings anywhere an enum would be needed; in that case, it must be the name of the enum's member."""
from . import metadata_description as desc
from .metadata_description import inf
import collections

class BuilderError(Exception):
    """An implementation detail. We want something specific to throw."""
    pass

def conv_enum(enum, name):
    """Used for enum parameter normalization."""
    if isinstance(name, str):
        return getattr(enum, name)
    return name

# The following are useful string constants for use with the string constant converting functionality:
constructor = "constructor"
constant = "constant"
specified = "specified"
writable = "writable"
readonly = "readonly"
varies = "varies"
dynamic = "dynamic"
# Start with p_ because some conflict with builtins.
p_boolean = "boolean"
p_int = "int"
p_float = "float"
p_double = "double"
p_float3 = "float3"
p_float6 = "float6"
p_int_array = "int_array"
p_float_array = "float_array"
p_buffer = "buffer"

class Builder:
    """Builds information for the bindings generator."""

    def __init__(self, c_info):
        self.c_info = c_info
        self.categories = dict()
        self.functions = dict()
        # We need to pre-register and annotate functions, as some functions are undocumented and uncategorized.
        for name in c_info['functions']:
            f = self._c_function_unvalidated(name = name, category = None, doc = None, param_docs = dict(), defaults = dict(), arrays = [])
            self.functions[name] = f
        self.nodes = dict()
        self.documented_enums = dict() # holds tuples (description, members) where members is a dict.


    def register_c_category(self, name, doc_name, doc_description):
        """Registers a category for C functions to be in."""
        cat = desc.FunctionCategory(name = name, doc_name = doc_name, doc_description = doc_description, functions = [])
        self.categories[name] = cat

    def c_function(self, name, category, doc, param_docs = dict(), defaults = dict(), arrays = []):
        """Amends functions, partially built from the header.

name: The name of the function to be amended.
category: the function's category for documentation purposes.
doc: the description of the function.
param_docs: A dict representing the documentation of each param.
defaults: A dict mapping params to defaults.
arrays: A list of tuples containing the names of parameters forming a (length, array) pair.
"""
        if category not in self.categories:
            raise BuilderError("Attempt to use category {} before registration.".format(category))
        if doc is None:
            raise BuilderError("Explicitly registered functions must always have documentation.")
        f = self._c_function_unvalidated(category = category, doc = doc, name = name, param_docs = param_docs,
            arrays = arrays, defaults = defaults)
        undocumented_params = []
        for i in f.params:
            if isinstance(i, desc.SimpleParam):
                if i.doc is None:
                    undocumented_params.append(i)
            else:
                if i.params[0].doc is None:
                    undocumented_params.append(i.params[0])
                if i.params[1].doc is None:
                    undocumented_params.append(i.params[1])
        if len(undocumented_params):
            undocumented_params = " ".join((i.name for i in undocumented_params))
            print("Warning: function {}: the following parameters are undocumented:\n".format(name))
            print(" "*2 + undocumented_params)
        self.functions[name] = f
        self.categories[category].functions.append(f)


    def _c_function_unvalidated(self, name, category, doc, param_docs, defaults, arrays):
        category = self.categories.get(category, None)
        info = self.c_info['functions'][name]
        info = self._convert_functioninfo(info)
        info.category = category
        # Annotate defaults and documentation.
        for p in info.params:
            if p.name in defaults:
                p.default = defaults[p.name]
                del defaults[p.name]
            if p.name in param_docs:
                p.doc = param_docs[p.name]
        if len(defaults):
            raise BuilderError("Defaults specified for parameters that don't exist: {}".format(" ".join(defaults.keys())))
        # Now we handle array params.
        complex_params = []
        for i, j in zip(info.params[:], info.params[1:]):
            if (i.name, j.name) in arrays:
                complex_params.append(desc.ArrayParam(params = (i, j)))
            else:
                complex_params.append(i) # But not j, because the next two might be an array parameter.
        # We might be one short.
        used = sum([i._uses for i in complex_params])
        info.params = complex_params + list(info.params[used:])
        return info

    def _convert_functioninfo(self, info):
        # Convert a FunctionInfo from get_info to a Function from bindings_description.
        # This is missing docs, etc, so we need to annotate it after return.
        # Engages with _convert_typeinfo to make a mutually recursive algorithm, that hits all FunctionInfos and TypeInfos involved; both functions are entry points.
        return_type = self._convert_typeinfo(info.return_type, translate_typedef = True)
        return_type_pretty = self._convert_typeinfo(info.return_type, translate_typedef = False)
        params = []
        for i in info.args:
            type = i.type
            paramname = i.name
            type_pretty = self._convert_typeinfo(type, translate_typedef = False)
            type = self._convert_typeinfo(type, translate_typedef = True)
            # This is just boring and straightforward normalization to prevent redundancies in the metadata.
            # We can often guess defaults.
            param_doc = None
            if paramname == "destination":
                param_doc = "Holds the result of a call to this function."
            elif paramname == "serverHandle":
                param_doc = "The server to manipulate."
            elif paramname == "nodeHandle":
                param_doc = "The node to manipulate."
            elif paramname == "bufferHandle":
                param_doc = "The buffer to manipulate."
            elif paramname == "propertyIndex":
                param_doc = "The property to manipulate."
            params.append(desc.SimpleParam(name = paramname, type = type, type_pretty = type_pretty, doc = param_doc))
        return desc.Function(doc = None, return_type = return_type, return_type_pretty = return_type_pretty,
            name = info.name, params = params, category = None)

    def _convert_typeinfo(self, type, translate_typedef):
        base = type.base
        indirection = type.indirection
        quals = type.quals
        if translate_typedef and base in self.c_info['typedefs']:
            td = self.c_info['typedefs'][base]
            indirection += td.indirection
            quals = {i[0]+td.indirection: i[1] for i in quals.items()}
            quals.update(td.quals)
            base = td.base
        if isinstance(base, desc.Function):
            base = self._convert_functioninfo(base)
        return desc.TypeInfo(base = base, indirection = indirection, quals = quals)

    def document_enum(self, name, doc, members):
        """Document an enum.

name: the name of the enum.
doc: The description of the enum.
members: A dict mapping member identifiers to descriptions.
"""
        if name not in self.c_info['constants_by_enum']:
            raise BuilderError("{} is not an enum.".format(name))
        for i in members.keys():
            if i not in self.c_info['constants_by_enum'][name]:
                raise BuilderError("{} is not a member of {}".format(i, name))
        if len(members) != len(self.c_info['constants_by_enum'][name]):
            missing = set(self.c_info.constants_by_enum[name].keys())
            missing -= set(members.keys())
            missing = " ".join(missing)
            raise BuilderError("You didn't document all the members. Missing {}".format(missing))
        tmp = dict()
        for name, value in self.c_info['constants_by_enum'][name].items():
            doc = members[name]
            tmp[name] = desc.EnumMember(name = name, doc = doc, value = value)
        members = tmp
        self.documented_enums[name] = desc.Enum(name = name, doc = doc, members = members)


    def node(self, components, identifier, doc_name, doc_description):
        """Registers a node.

Most parameters  are self-explanatory.  Components is a list of the return values from the following functions in this file. Do *not* reuse return values from these functions. Ever.

Inputs and outputs count as components. They have inferred indexes based on their order as declared in the components list.

identifier is the Lav_OBJTYPE_xxx constant's name as a string."""
        # Does identifier exist?
        if identifier not in self.c_info['constants'] and not identifier.startswith("Lav_OBJTYPE"):
            raise BuilderError("Node {} with identifier {}: not a valid identifier.".format(name, identifier))
        properties = [i for i in components if isinstance(i, desc.Property)]
        properties.sort(key = lambda p: p.name)
        callbacks = [i for i in components if isinstance(i, desc.Callback)]
        callbacks.sort(key = lambda c: c.name)
        extra_functions = [i for i in components if isinstance(i, desc.Function)]
        extra_functions.sort(key = lambda e: e.name)
        inputs = [i for i in components if isinstance(i, desc.Input)]
        outputs = [i for i in components if isinstance(i, desc.Output)]
        node = desc.Node(identifier = identifier, doc_name = doc_name, doc_description = doc_description,
            properties = properties, callbacks = callbacks, extra_functions = extra_functions,
            inputs = inputs, outputs = outputs)
        self.nodes[identifier] = node

    def _connection_builder(self, cls, doc, channel_type, channels):
        """Builds connections.  User code should not use this function directly."""
        conv_enum(desc.ChannelTypes, channel_type)
        return cls(doc = doc, channels = channels, channel_type = channel_type)

    def input(self, doc, channel_type, channels):
        """Builds an input. See docs in bindings_description.ConnectionPoint for parameters."""
        return self._connection_builder(cls = desc.Input, doc = doc, channel_type = channel_type, channels = channels)

    def output(self, doc, channel_type, channels):
        """Builds an output. See docs in bindings_description.ConnectionPoint for parameters."""
        return self._connection_builder(cls = desc.Output, doc = doc, channel_type = channel_type, channels = channels)

    def property(self, name, type, identifier, doc, default = None, default_type = None, range = None, range_type = None, access_type = None, access_type_notes = None, associated_enum = None,
    array_length_range = None, array_length_range_type = None):
        """Builds properties.  Parameters match bindings_description.Property but are validated.

This function offers the following conveniences:

If range is a 2-tuple, range_type is RangeTypes.specified.
If range is a string, range is RangeTypes.dynamic.
If range is exactly the string "constructor", range_type is RangeTypes.constructor.
These conveniences are extended to array length ranges.

If associated_enum is specified, range is ignored and forced to cover the whole enum.

If default is None, the default of this property is the appropriate default for the property's type.
Note that the default of properties using associated_enum does not support this convenience.
If default is exactly the string "constructor", then default_type is DefaultTypes.constructor.
Otherwise, the default takes on the value specified.
Default is ignored for buffer and array properties.
Defaults for arrays are complicated, and array properties are rare in practice; consequently array property defaults are set by and documented in node constructors.

If left alone, access_type defaults to writable.  If access_type_notes is provided, access_type is ignored and set to AccessTypes.varied.  You should only have to set it to AccessTypes.readonly"""
        # We normalize, then pass through to the Property class.
        type = conv_enum(desc.PropertyTypes, type)
        default_type = conv_enum(desc.DefaultTypes, default_type)
        range_type = conv_enum(desc.RangeTypes, range_type)
        array_length_range_type = conv_enum(desc.RangeTypes, array_length_range_type)
        access_type = conv_enum(desc.AccessTypes, access_type)
        if associated_enum and not isinstance(default, str):
            raise builderError("Use of associated_enum without specifying default, or default is not a string.")
        if associated_enum:
            if associated_enum not in self.c_info['constants_by_enum']:
                raise BuilderError("{} is not a valid enum.".format(assocaited_enuim))
            if default not in self.c_info['constants_by_enum'][associated_enum]:
                raise BuilderError("{} is not a constant of enum {}".format(default, associated_enum))
            range = (min(self.c_info['constants_by_enum'][associated_enum].values()), max(self.c_info['constants_by_enum'][associated_enum].values()))
        defaults = {desc.PropertyTypes.boolean: 0, desc.PropertyTypes.int: 0, desc.PropertyTypes.float: 0.0,
            desc.PropertyTypes.double: 0.0, desc.PropertyTypes.float3: (0.0, 0.0, 0.0),
            desc.PropertyTypes.float6: (0.0, 0.0, 0.0, 0.0, 0.0, 0.0)}
        if default == "constructor":
            default_type = DefaultTypes.constructor
            default = None
        elif isinstance(default, str):
            default_type = desc.DefaultTypes.constructor
        elif default is None and type in defaults:
            default = defaults[type]
        elif default is None and type not in {desc.PropertyTypes.buffer, desc.PropertyTypes.int_array, desc.PropertyTypes.float_array}:
            raise BuilderError("You need to specify a property default.")
        # Range, range_types.
        def normalize_range(range, range_type, fail_msg):
            if isinstance(range, tuple) and len(range) == 2:
                range_type = desc.RangeTypes.specified
            elif range == "constructor":
                range_type = RangeTypes.constructor
            elif isinstance(range_type, str):
                range_type = desc.RangeTypes.dynamic
            elif type in {desc.PropertyTypes.boolean, desc.PropertyTypes.buffer, desc.PropertyTypes.float3, desc.PropertyTypes.float6}:
                return (range, range_type)
            elif range is None or range_type is None:
                raise BuilderError(fail_msg)
            return (range, range_type)
        (range, range_type) = normalize_range(range, range_type, "You must specify range and range_type")
        if type in {desc.PropertyTypes.int_array, desc.PropertyTypes.float_array}:
            (array_length_range, array_length_range_type) = normalize_range(array_length_range, array_length_range_type, "You must specify both array_length_range and array_length_range_type")
        if access_type_notes is not None:
            access_type = desc.AccessTypes.varied
        elif access_type is None:
            access_type = desc.AccessTypes.writable
        # that's it, finally. So...
        return desc.Property(name = name, identifier = identifier, doc = doc, type = type,
            default = default, default_type = default_type,
            range = range, range_type = range_type,
            array_length_range = array_length_range, array_length_range_type = array_length_range_type,
            access_type = access_type, access_type_notes = access_type_notes, associated_enum = associated_enum)

    def extra_function(self, doc, function, param_docs = dict(), defaults = dict(), arrays = []):
        """Return a function object suitable to represent a node's extra function.

The parameters here are the same as for c_function, save category which is not applicable."""
        if function not in self.c_info['functions']:
            raise ValueError("{} is not a valid function.".format(name))
        return self._c_function_unvalidated(name = function, doc = doc, category = None, param_docs = param_docs, defaults = defaults, arrays = arrays)

    def callback(self, name, setter, in_audio_thread, doc):
        """Make a callback.

Setter  is the setter function name for the callback, of the form Lav_blaNodeSetCallback.  We get the type of the function pointer from the second parameter of the setter callback.

in_audio_thread and doc are as in bindings_description."""
        # The  setter is registerd, but doesn't necessarily contain docs.
        if setter not in self.functions:
            raise BuilderError("Setter {} does not exist.".format(setter))
        setter = self.functions[setter]
        # The signature is the converted TypeInfo of the setter's second parameter's base.
        signature = setter.params[1].type.base
        signature_typedef = setter.params[1].type_pretty.base
        return desc.Callback(name = name, doc = doc, setter = setter, signature =signature, signature_typedef = signature_typedef, in_audio_thread = in_audio_thread)

    def finish(self):
        """Compile the info we have collected into a metadata_description.Metadata instance."""
        nodes = self.nodes
        functions = self.functions
        function_categories = self.categories
        for i in function_categories.values():
            i.functions.sort(key = lambda x: x.name)
        # We have a number of enum instances already, but need to make up the rest here.
        undocumented_enums = dict()
        for i, j in self.c_info['constants_by_enum'].items():
            if i in self.documented_enums:
                continue
            members = {k[0]: desc.EnumMember(name = k[0], value = k[1], doc = None) for k in j.items()}
            undocumented_enums[i] = desc.Enum(name = i, doc = None, members = members)
        enums = dict(self.documented_enums)
        enums.update(undocumented_enums)
        typedefs = collections.OrderedDict([(i[0], self._convert_typeinfo(i[1], translate_typedef = False)) for i in self.c_info['typedefs'].items()])
        return desc.Metadata(nodes = nodes, functions = functions, function_categories = function_categories,
            enums = enums, documented_enums = dict(self.documented_enums), typedefs = typedefs)

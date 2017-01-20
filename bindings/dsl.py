"""This is the bindings DSL.

An instance of Builder is created by the bindings generator. Then, all .py files in metadata/* are read into strings and executed with the methods of the Builder instance and contents of the bindings_description module in the global namespace.

As a convenience, this module accepts strings anywhere an enum would be needed; in that case, it must be the name of the enum's member."""
from . import bindings_description as desc
from desc import inf

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
            self._c_function_unvalidated(name = name, category = None, doc = None, param_docs = dict(), defaults = dict(), arrays = [], _is_preregister = Trueq)

    def register_c_category(self, name, doc_name, doc_description):
        """Registers a category for C functions to be in."""
        cat = desc.FunctionCategory(name = name, doc_name = doc_name, doc_description = doc_description)
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
        if name not in self.c_info['functions']:
            raise ValueError("{} is not a valid function.".format(name))
        if category not in self.categories:
            raiseValueError("Attempt to use category {} before registering it. Categories must be registered first.".format(category))
        self._c_function_unvalidated(name = name, doc = doc, category = category, param_docs = param_docs, defaults = defaults, arrays = arrays)

    def _c_function_unvalidated(self, name, category, doc, param_docs, defaults, arrays, _is_preregister = False):
        category = self.categories.get(category, None)
        info = self.c_info['functions'][name]
        # info is the version of the class defined in get_info, i.e. the one that only knows about C stuff.
        return_type = self._convert_typeinfo(info.return_type, translate_typedef = True)
        return_type_pretty = self._convert_typeinfo(info.return_type, translate_typedef = False)
        # Build the simple parameters, we'll handle making them into arrays later.
        params = []
        for i in info.args:
            type = i.type
            paramname = i.name
            type_pretty = self._convert_typeinfo(type, translate_typedef = False)
            type = self._convert_typeinfo(type, translate_typedef = True)
            # This is just boring and straightforward normalization to prevent redundancies in the metadata.
            # If docs aren't present, we can guess a default. Most functions have a parameter that hits one of these special cases.
            if paramname in param_docs:
                param_doc = param_docs[paramname]
            elif paramname == "destination":
                param_doc = "Holds the result of a call to this function."
            elif paramname == "serverHandle":
                param_doc = "The server to manipulate."
            elif paramname == "nodeHandle":
                param_doc = "The node to manipulate."
            elif paramname == "bufferHandle":
                param_doc = "The buffer to manipulate."
            elif paramname == "propertyIndex":
                param_doc = "The property to manipulate."
            else:
                if not _is_preregister:
                    print("Warning: param {} of function {} is not documented.".format(paramname, name))
                param_doc = "This parameter is undocumented."
            params.append(desc.SimpleParam(name = name, type = type, type_pretty = type_pretty, doc = param_doc))
        # Now we handle array params.
        complex_params = []
        for i, j in zip(params[:], params[1:]):
            if (i.name, j.name) in arrays:
                complex_params.append(desc.ArrayParam(params = (i, j)))
            else:
                complex_params.append(i) # But not j, because the next two might be an array parameter.
        # We might be one short.
        used = sum([i._uses for i in complex_params])
        params = complex_params + params[used:]
        # Now, we can finally build the function itself.
        func = desc.FunctionInfo(doc = doc, return_type = return_type, return_type_pretty = return_type_pretty,
            name = name, params = params, category = category)
        self.functions[name] = func

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
        return desc.TypeInfo(base = base, indirection = indirection, quals = quals)

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
        outputs = [i for i in components if isinstance(i, desc.Outputs)]
        node = desc.Node(identifier = identifier, name = name, doc_name = doc_name, doc_description = doc_description,
            properties = properties, callbacks = callbacks, extra_functions = extra_functions,
            inputs = inputs, outputs = outputs)
        #todo: put this somewhere.

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

    def property(self, name, type, identifier, doc, default = None, default_type = None, range = None, range_type = None, access_type = None, access_type_notes = None, associated_enum = None):
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
            range = min(self.c_info['constants_by_enum'][associated_enum].values()), max(self.c_info['constants_byu_enum'][associated_enum].values()))
        defaults = {desc.PropertyTypes.int: 0, desc.PropertyTypes.float: 0.0,
            desc.PropertyTypes.double: 0.0, desc.PropertyTypes.float3: (0.0, 0.0, 0.0),
            desc.PropertyTypes.float6: (0.0, 0.0, 0.0, 0.0, 0.0, 0.0)}
        if default == "constructor":
            default_type = DefaultTypes.constructor
            default = None
        elif isinstance(default, str):
            default_type = desc.DefaultTypes.constructor
        elif default is None and type in defaults:
            default = defaults[type]
        elif type not in {PropertyTypes.buffer, PropertyTypes.int_array, PropertyTypes.float_array}:
            raise BuilderError("You need to specify a property default.")
        # Range, range_types.
        def normalize_range(range, range_type, fail_msg):
            if isinstance(range, tuple) and len(range) == 2:
                range_type = RangeTypes.specified
            elif range == "constructor":
                range_type = RangeTypes.constructor
            elif isinstance(range_type, str):
                range_type = desc.RangeTypes.dynamic
            elif range is None or range_type is None:
                raise BuilderError(fail_msg)
            return (range, range_type)
        (range, range_type) = normalize_range(range, range_type, "You must specify range and range_type")
        (array_length_range, array_length_range_type) = normalize_range(array_length_range, array_length_range_type, "You must specify both array_length_range and array_length_range_type")
        if access_type_notes is not None:
            access_type = desc.AccessTypes.varied
        elif access_type is None:
            access_type = AccessTypes.writable
        # that's it, finally. So...
        return desc.Property(name = name, identifier = identifier, doc = doc, type = type,
            default = default, default_type = default_type,
            range = range, range_type = range_type,
            array_length_range = array_length_range, array_length_range_type = array_length_range_type,
            access_type = access_type, access_type_notes = access_type_notes, associated_enum = associated_enum)

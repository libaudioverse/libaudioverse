"""This is the bindings DSL.

An instance of Builder is created by the bindings generator. Then, all .py files in metadata/* are read into strings and executed with the methods of Builder in the global namespace."""
from . import bindings_description as desc

class Builder:
    """Builds information for the bindings generator."""

    def __init__(self, c_info):
        self.c_info = c_info
        self.categories = dict()
        self.functions = dict()

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
        category = self.categories[category]
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
            if paramname in param_docs:
                doc = param_docs[paramname]
            elif paramname == "destination":
                doc = "Holds the result of a call to this function."
            elif paramname == "serverHandle":
                doc = "The server to manipulate."
            elif paramname == "nodeHandle":
                doc = "The node to manipulate."
            elif paramname == "bufferHandle":
                doc = "The buffer to manipulate."
            elif paramname == "propertyIndex":
                doc = "The property to manipulate."
            else:
                print("Warning: param {} of function {} is not documented.".format(paramname, name))
                doc = "This parameter is undocumented."
            params.append(desc.SimpleParam(name = name, type = type, type_pretty = type_pretty, doc_description = doc))
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
        func = desc.FunctionInfo(return_type = return_type, return_type_pretty = return_type_pretty,
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

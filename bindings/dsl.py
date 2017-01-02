""This is the bindings DSL.

An instance of Builder is created by the bindings generator. Then, all .py files in metadata/* are read into strings and executed with the methods of Builder in the global namespace."""

class Builder:
    """Builds information for the bindings generator."""

    def register_c_category(self, name, description):
        ""Registers a category for C functions to be in."""
        pass

    def c_function(self, name, doc, param_docs, defaults):
        """Amends functions, partially built from the header.

name: The name of the function to be amended.
doc: the description of the function.
param_docs: A dict representing the documentation of each param.
defaults: A dict mapping params to defaults.

The keys of param_docs are either single parameter names or tuples (length, pointer).  In the latter case, the parameter is interpreted as an array.
"""
    pass

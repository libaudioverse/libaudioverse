"""This is the bindings DSL.

An instance of Builder is created by the bindings generator. Then, all .py files in metadata/* are read into strings and executed with the methods of Builder in the global namespace."""

class Builder:
    """Builds information for the bindings generator."""

    def __init__(self):
        self.categories = []
        self.functions = []

    def register_c_category(self, name, doc_name, doc_description):
        """Registers a category for C functions to be in."""
        self.categories.append((name, doc_name, doc_description))

    def c_function(self, name, category, doc, param_docs = dict(), defaults = dict()):
        """Amends functions, partially built from the header.

name: The name of the function to be amended.
category: the function's category for documentation purposes.
doc: the description of the function.
param_docs: A dict representing the documentation of each param.
defaults: A dict mapping params to defaults.

The keys of param_docs are either single parameter names or tuples (length, pointer).  In the latter case, the parameter is interpreted as an array.
"""
        self.functions.append((name, category, doc, param_docs, defaults))

"""This is the base class for all bindings generator backends.

See also metadata_description.py, which defines the classes that represent Libaudioverse objects. Your backend will receive instances of them.
"""
from abc import abstractmethod

class BackendBase(metaclass = abc.ABCmeta):
    """A bindings generator backend.

This class defines a number of methods which are called by the bindings generator, Jinja2 filters, and some utilities.
visit_ methods are called on individual objects.
begin_ and end_methods are called to begin/end sections.
Functions are called by the generator in the order specified in this file.

You need to use the API for writing files that is provided by this class.  Libaudioverse may do dry runs of bindings generation for continuous integration or otherwise send the files to places that are not the hard disk.

This is an abstract class. You must implement all abstract methods.

This class provides a Jinja2 context as the context member and a render helper method.
Any function beginning with jfilt_ will be inserted as a filter  and any function beginning with jfn_ as a function.
You must implement the minimal filters provided here, but may provide more.
Unless disabled, all text files you write will be passed through Jinja2 after generation and before going to disk.

The transformers in transformers.py are always available to your templates under their names in that file.
You can override one here with the jfilt_ mechanism. Don't.
"""

    def __init__():
        # Todo: set up Jinja2 contexts.
        pass

    @abstractmethod
    def begin_c(self):
        """Called to begin the traversal of the information extracted from the C header."""
        pass

    @abstractmethod
    def visit_enum(self, info):
        """Called on all enums."""
        pass

    @abstractmethod
    def visit_typedef(self, name, type):
        """Called on all typedefs."""
        pass

    @abstractmethod
    def visit_error_enum(self, info):
        """Visit the error enum to translate values to exceptions or similar."""
        pass

    @abstractmethod
    def visit_c_function(self, info):
        """Called on all functions in the C header."""
        pass

    @abstractmethod
    def end_c(self):
        """End traversal of information extracted from the C header."""
        pass

    @abstractmethod
    def write_manually_bound_objects(self):
        """Write all objects that this generator backend manually binds, i.e. servers, buffers, and classes to represent properties."""
        pass

    @abstractmethod
    def begin_nodes(self):
        """We will now traverse all nodes, in alphabetical order by name."""
        pass

    @abstractmethod
    def begin_node(self, node_info):
        """Begin binding a specific node."""
        pass

    @abstractmethod
    def begin_properties(self, node_info):
        """Begin binding properties."""
        pass

    @abstractmethod
    def visit_property(self, node_info, property_info):
        """Bind a property."""
        pass

    @abstractmethod
    def end_properties(self, node_info):
        """We're done with properties."""
        pass

    @abstreactmethod
    def begin_callbacks(self, node_info):
        """We're going to bind all the callbacks."""
        pass

    @abstractmethod
    def visit_callback(self, node_info, callback_info):
        """Visit a callback."""
        pass

    @abstractmethod
    def end_callbacks(self, node_info):
        """We're done binding callbacks."""
        pass


    @abstractmethod
    def begin_extra_functions(self, node_info):
        """Visit this node's extra functions."""
        pass

    @abstractmethod
    def visit_extra_function(self, node_info, function_info):
        """Bind an extra function."""
        pass

    @abstractmethod
    def end_extra_function(self, node_info):
        """End binding of extra functions."""
        pass

    @abstractmethod
    def end_node(self, info):
        """We're done binding a specific node."""
        pass

    @abstractmethod
    def end_nodes(self):
        """We are done traversing nodes. This is the end of bindings generation."""
        pass

    # Utility functions.

    def prepopulate(self, path):
        """Prepopulate this bindings generator with the contents of a directory.

The path is relative to the location of your backend.  After this function is called, requests for specific files will prepopulate the CodeBuilder as if you had read them and added the lines yourself.

Do not use this on a directory containing binary files."""
    pass

    def get_builder(self, path):
        """Given a path, create a CodeBuilder."""
        pass

class CodeBuilder:
    """This class represents a destination for code.  You need to use it instead of files.  Instantiate it with get_builder.

As a convenience, lines can be appended with +=.  If you append more than one line, all lines are indented to the current level."""

    def __init__(self, backend, path, indent = "    "):
        self.backend = backend.
        self.path = path
        if path not in self.backend._code_files:
            self.backend._code_files[path] = []
        self._text = self.backend._code_files
        self._indent_level = 0
        self._indent = indent

    def write(self, text, allow_indent = True):
        text = text.replace("\r", "").split("\n")
        for l in text:
            if allow_indent and len(line):
                line = self._indent*self._indent_level + line
            self._text.append(line)

    def __iadd__(self, other):
        """Convenient way to add lines."""
        self.write(other)

    def indent(self):
        self._indent += 1

    def outdent(self):
        self._indent -= 1
        if self._indent < 0:
            raise ValueError("Attempt to outdent more than you indented.")

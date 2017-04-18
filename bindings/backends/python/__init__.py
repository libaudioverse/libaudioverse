import bindings.backend_base
import bindings.metadata_description as desc


class Backend(bindings.backend_base.BackendBase):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def build(self):
        pass

    def jfilt_codelit(self, text):
        """Convert code literals."""
        return text

    def jfunc_propref(self, objname, propname):
        """Return text referencing a property."""
        return objname

    def jfilt_node(self, nodename):
        """Reference a node."""
        return nodename

    def jfilt_math(self, text):
        """Takes LaTeX math and marks it up for the final output format."""
        return text

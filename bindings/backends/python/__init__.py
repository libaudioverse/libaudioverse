import bindings.backend_base
import bindings.metadata_description as desc


class Backend(bindings.backend_base.BackendBase):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def visit_enum(self, info):
        pass

    def visit_typedef(self, name, type):
        pass

    def visit_error_enum(self, info):
        pass

    def visit_c_function(self, info):
        pass

    def begin_node(self,     node_info):
        pass

    def begin_properties(self, node_info):
        pass

    def visit_property(self, node_info, property_info):
        pass

    def end_properties(self, node_info):
        pass

    def begin_callbacks(self, node_info):
        pass

    def visit_callback(self, node_info, callback_info):
        pass

    def end_callbacks(self, node_info):
        pass

    def begin_extra_functions(self, node_info):
        pass

    def visit_extra_function(self, node_info, function_info):
        pass

    def end_extra_functions(self, node_info):
        pass

    def end_node(self, info):
        pass

    def build(self):
        pass

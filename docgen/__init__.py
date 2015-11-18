import os.path
import os
import sys
repository_root = os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]
sys.path = [repository_root] + sys.path
import bindings.doc_helper
from .c_api import make_c_api
from .node_reference import make_node_reference
from .enumerations import make_enumerations
from . import asciidoc_filters

def prepare_docs(all_info):
    bindings.doc_helper.prepare_docs(all_info,
    enum =asciidoc_filters.enum, node = asciidoc_filters.node, param = asciidoc_filters.param, codelit=asciidoc_filters.codelit,
    latex = asciidoc_filters.latex)

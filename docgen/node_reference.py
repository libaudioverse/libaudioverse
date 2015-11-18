from bindings import get_info, transformers, doc_helper
import jinja2
from . import asciidoc_filters

def make_node_reference(all_info):
    env = jinja2.Environment(loader = jinja2.PackageLoader(__package__, ""), undefined = jinja2.StrictUndefined, trim_blocks = True)
    env.filters.update(transformers.get_jinja2_filters(all_info))
    env.filters.update(asciidoc_filters.bound_filters(all_info))
    context = dict()
    context.update(all_info)
    template = env.get_template("node_reference.t")
    sorted_nodes= list(context['metadata']['nodes'].items())
    sorted_nodes.sort(key = lambda x: x[1]['doc_name'].lower())
    sorted_nodes= [i[0] for i in sorted_nodes]
    sorted_nodes.remove('Lav_OBJTYPE_GENERIC_NODE')
    sorted_nodes = ['Lav_OBJTYPE_GENERIC_NODE'] + sorted_nodes
    context['sorted_nodes'] = sorted_nodes
    context['nodes'] = context['metadata']['nodes']
    return template.render(context)

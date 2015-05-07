import os.path
import os
import sys
repository_root = os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]
sys.path = [repository_root] + sys.path
from bindings import get_info, transformers
import jinja2

from .c_api import make_c_api

def make_property_table():
	env = jinja2.Environment(loader = jinja2.PackageLoader(__package__, ""), undefined = jinja2.StrictUndefined, trim_blocks = True)
	env.filters.update(transformers.get_jinja2_filters())
	context = dict()
	all_info =get_info.get_all_info()
	context.update(all_info)
	template = env.get_template("object_reference.t")
	sorted_nodes= context['metadata']['nodes'].items()
	sorted_nodes.sort(key = lambda x: x[1]['doc_name'].lower())
	sorted_nodes= [i[0] for i in sorted_nodes]
	sorted_nodes.remove('Lav_OBJTYPE_GENERIC_NODE')
	sorted_nodes = ['Lav_OBJTYPE_GENERIC_NODE'] + sorted_nodes
	context['sorted_nodes'] = sorted_nodes
	context['nodes'] = context['metadata']['nodes']
	return template.render(context)


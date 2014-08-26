import os.path
import os
import sys
repository_root = os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]
sys.path = [repository_root] + sys.path
from bindings import get_info, transformers
import jinja2

def make_property_table():
	env = jinja2.Environment(loader = jinja2.PackageLoader(__package__, ""), undefined = jinja2.StrictUndefined, trim_blocks = True)
	env.filters.update(transformers.get_jinja2_filters())
	context = dict()
	context.update(get_info.all_info)
	template = env.get_template("object_reference.t")
	sorted_objects = context['metadata'].items()
	sorted_objects.sort(key = lambda x: x[1]['doc_name'].lower())
	sorted_objects = [i[0] for i in sorted_objects]
	sorted_objects.remove('Lav_OBJTYPE_GENERIC')
	sorted_objects = ['Lav_OBJTYPE_GENERIC'] + sorted_objects
	context['sorted_objects'] = sorted_objects
	return template.render(context)

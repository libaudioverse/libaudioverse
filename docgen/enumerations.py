from bindings import get_info, doc_helper, transformers
from . import asciidoc_filters
import jinja2
import yaml

def make_enumerations():
	all_info =get_info.get_all_info()
	doc_helper.prepare_docs(all_info,
		param = asciidoc_filters.param, node = asciidoc_filters.node, enum = asciidoc_filters.enum, codelit=asciidoc_filters.codelit)
	env = jinja2.Environment(loader=jinja2.PackageLoader(__package__, ""), undefined=jinja2.StrictUndefined, trim_blocks=True)
	env.filters.update(transformers.get_jinja2_filters(all_info))
	context=dict()
	context.update(all_info)
	template=env.get_template("enumerations.t")
	return template.render(context)

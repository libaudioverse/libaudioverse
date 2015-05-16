import jinja2

def identity(all_info, x):
	return x

def prepare_docs(all_info,
param = identity, node = identity, enum = identity,
no_doc_description = "No description defined."):
	"""Runs over the all_info specified.
Renders all documentation therein through Jinja2, using the filters passed to this function.
Note that no globals are provided and the default for all arguments is to do nothing to the text.
Furthermore, some "missing" fields such as doc_description are added by this function.  Do all desired error checking first.

The filters must accept two arguments.  The first must be an all_info dict.  This dict may be partially modified by this function.
The second is the string to transform."""
	env=jinja2.Environment(undefined=jinja2.StrictUndefined)
	env.filters['param'] = lambda x: param(all_info, x)
	env.filters['node'] = lambda x: node(all_info, x)
	env.filters['enum'] = lambda x: enum(all_info, x)
	#yes, this is a local function.
	def render(s):
		template=env.from_string(s)
		return template.render()
	for i in all_info['metadata']['functions'].itervalues():
		i['doc_description'] = render(i.get('doc_description', no_doc_description))
		if 'params' in i: #some functions don't have params
			for n, p in i['params'].iteritems():
				i['params'][n] = render(p)
	for i in all_info['metadata']['nodes'].itervalues():
		i['doc_description'] = render(i.get('doc_description', no_doc_description))
		constructor=i.get('constructor', dict())
		constructor['doc_description'] = render(constructor.get('doc_description', no_doc_description))
		constructor['params'] = constructor.get('params', dict())
		for n, p in constructor['params'].iteritems():
			constructor['params'][n] = render(p)
	#no return, we modify in place.

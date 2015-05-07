from bindings import get_info, transformers
from bindings import metadata_handler
import jinja2
import yaml

def type_to_string(type, so_far= None, indirection =0, typedefs =  None, apply_typedefs=False):
	#Special case of pure typedef, which is the case in which we are most interested in expanding:
	if apply_typedefs and type.base in typedefs and type.indirection == 0:
		return type_to_string(typedefs[type.base])
	#Special case of function pointers:
	if isinstance(type.base, get_info.FunctionInfo) and type.indirection == 1:
		return handle_function_pointer(type.base)
	#first, the level 0 qualifiers.
	q= type.quals.get(indirection, [])
	if len(q):
		qualstring=" ".join(q)+" "
	else:
		qualstring=""
	if so_far is None:
		return type_to_string(type, qualstring+type.base, type.indirection)
	if indirection == 0: #We've come full circle and/or had no indirection to start with.
		return so_far
	#append a * and the qualstring.
	so_far+="*"
	if len(qualstring):
		so_far+=" "+qualstring
	return type_to_string(type, so_far, indirection-1)

def handle_function_pointer(f):
	return function_to_string(f, True)

def function_to_string(f, is_pointer = False):
	ret_string = type_to_string(f.return_type)+" "
	if is_pointer:
		name_string = "("+f.name+"*)"
	else:
		name_string=f.name
	arg_strings = []
	for i in f.args:
		s=type_to_string(i.type)+" "+i.name
		arg_strings.append(s)
	arg_string="("+", ".join(arg_strings)+")"
	return ret_string+name_string+arg_string

def compute_involved_typedefs(f, all_info):
	retval = []
	for i in [f.return_type]+[q.type for q in f.args]:
		if i.base == 'LavError' or i.base == 'LavHandle':
			continue
		if i.base in all_info['typedefs']:
			retval.append(i.base)
	retval.sort()
	for i, j in enumerate(retval):
		retval[i] = j, all_info['typedefs'][j]
	return retval

def verify_all_parameters_documented(info, docs):
	for func in info['functions'].itervalues():
		if func.name not in docs:
			continue
		if len(func.args) and 'params' not in docs[func.name]:
			raise ValueError("{} has parameters but no documentation for them.".format(func.name))
		for p in func.args:
			if p.name not in docs[func.name]['params']:
				raise ValueError("{}: undocumented param {}".format(func.name, p.name))

def make_c_api():
	all_info =get_info.get_all_info()
	env = jinja2.Environment(loader=jinja2.PackageLoader(__package__, ""), undefined=jinja2.StrictUndefined, trim_blocks=True)
	env.filters.update(transformers.get_jinja2_filters())
	env.filters['prototype']=function_to_string
	env.filters['type_to_string'] = type_to_string
	env.filters['compute_involved_typedefs'] = lambda f: compute_involved_typedefs(f, all_info)
	context=dict()
	context.update(all_info)
	functions_by_category=dict()
	for i in all_info['metadata']['function_categories']:
		functions_by_category[i['name']] =[]
	for n, i in all_info['metadata']['functions'].iteritems():
		category=i['category']
		if category not in functions_by_category:
			raise ValueError("{}: {} is not a valid category".format(n, category))
		functions_by_category[category].append(n)
	for i, j in functions_by_category.iteritems():
		j.sort() #alphabetize all of them
	context['functions_by_category'] = functions_by_category
	template=env.get_template("c_api.t")
	verify_all_parameters_documented(all_info, all_info['metadata']['functions'])
	return template.render(context)

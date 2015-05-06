from bindings import get_info

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

def make_function_reference():
	pass
	
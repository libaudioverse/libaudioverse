def enum(all_info, x):
	#reference an enum. For now, we don't have an enum section.
	return x

def node(all_info, x):
	#reference a node.
	if x not in all_info['metadata']['nodes']:
		raise ValueError("{} is not a valid node".format(x))
	link_text = all_info['metadata']['nodes'][x]['doc_name'] + " node"
	link_id = "node-{}".format(x)
	return "<<{},{}>>".format(link_id, link_text)

def param(all_info, x):
	#This is identity.
	return x

def codelit(all_info, x):
	return "`{}`".format(x)

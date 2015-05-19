import yaml
import glob
import os
import os.path
import re

def cleanup_function(all_info, name, d):
	"""This function ignores category so we can use it on node extra functions."""
	if name not in all_info['functions']:
		raise ValueError("{} is not a valid function.".format(name))
	func =all_info['functions'][name]
	if 'params' not in d:
		d['params'] = dict()
	for p in func.args:
		if p.name not in d['params']:
			if p.name == "destination":
				d['params'][p.name] = "Holds the result of a call to this function."
			elif p.name == "simulationHandle":
				d['params'][p.name] = "The simulation to manipulate."
			elif p.name == "propertyIndex":
				d['params'][p.name] = "The property to manipulate."
			elif p.name == "nodeHandle":
				d['params'][p.name] = "The node to manipulate."

def cleanup_property(i):
	i['doc_description'] = i.get('doc_description', 'No description available.')
	i['is_dynamic'] = i.get('is_dynamic', False)
	i['read_only'] = i.get('read_only', False)

def cleanup_callback(i):
	pass

def cleanup_event(i):
	i['doc_description'] = i.get('doc_description', 'No description available.')
	i['multifiring_protection'] = i.get('multifiring_protection', False)

def cleanup_extra_function(all_info, name, d, for_node):
	d['doc_description'] = d.get('doc_description', 'No description available')
	cleanup_function(all_info, name, d) #add parameters, etc.
	#Build a name token that bindings generators can work off.
	prefix_tokens=for_node[len("Lav_OBJTYPE_"):].split("_")
	prefix = "".join([i[0].upper()+i[1:].lower() for i in prefix_tokens])
	prefix = "Lav_"+prefix[0].lower()+prefix[1:]
	if not name.startswith(prefix):
		raise ValueError("{} not prefix of {}".format(prefix, name))
	convenient_name = name[len(prefix):]
	#Importing the camelcase_to_underscores transformer from the transformers module causes circular imports. Todo: fix.
	convenient_name=convenient_name[0].lower()+convenient_name[1:]
	convenient_name=re.sub('[A-Z]', lambda x: '_'+x.group(0).lower(), convenient_name)
	d['name'] = convenient_name

def cleanup_node(all_info, name, node):
	"""Ensures that a node has all of the properties which nodes must have."""
	node['has_properties'] = True
	node['has_events'] = True
	node['has_extra_functions'] = True
	node['has_callbacks'] = True
	if 'properties' not in node:
		node['has_properties'] = False
		node['properties'] = dict() #empty properties.
	if 'events' not in node:
		node['has_events'] = False
		node['events'] = dict()
	if 'extra_functions' not in node:
		node['has_extra_functions'] = False
		node['extra_functions'] = dict()
	if 'callbacks' not in node:
		node['has_callbacks'] = False
		node['callbacks'] = list()
	for i in node['properties'].itervalues():
		cleanup_property(i)
	for i in node['events'].itervalues():
		cleanup_event(i)
	for i in node['callbacks']:
		cleanup_callback(i)
	for n, d in node['extra_functions'].iteritems():
		cleanup_extra_function(all_info, n, d, name)
	node['doc_description'] = node.get('doc_description', 'No description available.')
	node['doc_name'] = node.get('doc_name', 'No document name available')
	#Build the name of the function that acts as this node's constructor.
	constructor_tokens = name[len("Lav_OBJTYPE_"):-len("_NODE")].split("_")
	constructor_tokens = [i.lower() for i in constructor_tokens]
	constructor_tokens=[i[0].upper()+i[1:] for i in constructor_tokens]
	constructor= "Lav_create"+"".join(constructor_tokens)+"Node"
	node['constructor'] = constructor

def load_node(all_info, path):
	with file(path) as f:
		node_info = yaml.load(f)
	name =os.path.split(path)[1][:-len(".y")]
	cleanup_node(all_info, name, node_info)
	return (name, node_info)

def make_metadata(all_info):
	#Compute where the metadata is so we can load it.
	metadata_dir = os.path.abspath(os.path.join(
	os.path.split(os.path.abspath(__file__))[0],
	"..",
	"metadata"
	))

	#First, build the base dict. This is metada.yaml:
	with file(os.path.join(metadata_dir, "metadata.y")) as f:
		metadata = yaml.load(f)

	#Next, we add all the files in the nodes subdirectory:
	nodes = glob.glob(os.path.join(metadata_dir, "nodes", "*.y"))
	nodes= [os.path.abspath(i) for i in nodes]
	metadata['nodes'] = dict()

	for i in nodes:
		node_info = load_node(all_info, i)
		metadata['nodes'][node_info[0]] = node_info[1]
	#Grab function categories and function documentation:
	with file(os.path.join(metadata_dir, 'function_categories.y')) as f:
		metadata.update(yaml.load(f))
	with file(os.path.join(metadata_dir, 'functions.y')) as f:
		metadata.update(yaml.load(f))

	#run over all the functions, cleaning them up:
	for name, func in metadata['functions'].iteritems():
		cleanup_function(all_info, name, func)

	return metadata
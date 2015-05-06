"""This module exports one variable, metadata, a dict containing the combined metadata from all metadata files."""
import yaml
import glob
import os
import os.path

def cleanup_property(i):
	i['doc_description'] = i.get('doc_description', 'No description available.')

def cleanup_callback(i):
	pass

def cleanup_event(i):
	i['doc_description'] = i.get('doc_description', 'No description available.')
	i['multifiring_protection'] = i.get('multifiring_protection', False)

def cleanup_extra_function(i):
	i['doc_description'] = i.get('doc_description', 'No description available')

def cleanup_node(node):
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
	for i in node['extra_functions'].itervalues():
		cleanup_extra_function(i)
	node['doc_description'] = node.get('doc_description', 'No description available.')
	node['doc_name'] = node.get('doc_name', 'No document name available')

def load_node(path):
	with file(path) as f:
		node_info = yaml.load(f)
	cleanup_node(node_info)
	name =os.path.split(path)[1][:-len(".y")]
	return (name, node_info)

def make_metadata():
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
		node_info = load_node(i)
		metadata['nodes'][node_info[0]] = node_info[1]

	return metadata

"""This module exports one variable, metadata, a dict containing the combined metadata from all metadata files."""
import yaml
import glob
import os
import os.path

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
		with file(i) as f:
			node_info = yaml.load(f)
		name =os.path.split(i)[1][:-len(".y")]
		metadata['nodes'][name] = node_info

	return metadata

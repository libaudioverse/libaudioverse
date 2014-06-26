from . import get_info
from .python import make_python
import os.path
import os
import shutil

generators = {
'python' : make_python,
}

def write_files(files, binding_dir):
	if os.path.exists(binding_dir):
		shutil.rmtree(binding_dir)
	for name, contents in files.iteritems():
		fullpath = os.path.join(binding_dir, name)
		dir = os.path.split(fullpath)[0]
		if not os.path.exists(dir):
			os.makedirs(dir)
		with file(fullpath, 'w') as f:
			f.write(contents)
	shutil.copy(os.path.join(get_info.root_directory, 'build', 'libaudioverse.dll'), os.path.join(binding_dir, 'libaudioverse.dll'))

def make_bindings():
	for name, func in generators.iteritems():
		files = func(get_info.all_info)
		write_files(files, os.path.join(get_info.root_directory, 'build', 'bindings', name))

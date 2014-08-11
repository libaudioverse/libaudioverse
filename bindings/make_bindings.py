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
		if name == 'dll_location': #the special key that points at where we want the dll to go.
			path = os.path.join(binding_dir, contents)
			dir = os.path.split(path)[0]
			if not os.path.exists(dir):
				os.makedirs(dir)
			shutil.copy(os.path.join(get_info.root_directory, 'build', 'libaudioverse.dll'), path)
			continue
		fullpath = os.path.join(binding_dir, name)
		dir = os.path.split(fullpath)[0]
		if not os.path.exists(dir):
			os.makedirs(dir)
		with file(fullpath, 'w') as f:
			f.write(contents)

def make_bindings():
	for name, func in generators.iteritems():
		files = func(get_info.all_info)
		write_files(files, os.path.join(get_info.root_directory, 'build', 'bindings', name))

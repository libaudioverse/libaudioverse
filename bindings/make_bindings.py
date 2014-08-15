from . import get_info
from .python import make_python
import os.path
import os
import shutil

generators = {
'python' : make_python,
}

def write_files(files, source_dir, dest_dir):
	special_keys = ['dll_location', 'additional_directories']
	if os.path.exists(dest_dir):
		shutil.rmtree(dest_dir)
	for name, contents in files.iteritems():
		if name in special_keys:
			continue
		fullpath = os.path.join(dest_dir, name)
		dir = os.path.split(fullpath)[0]
		if not os.path.exists(dir):
			os.makedirs(dir)
		with file(fullpath, 'w') as f:
			f.write(contents)
	#handle the dll's copying.
	dll_target = files.get('dll_location', 'libaudioverse.dll')
	path = os.path.join(dest_dir, dll_target)
	dir = os.path.split(path)[0]
	if not os.path.exists(dir):
		os.makedirs(dir)
	shutil.copy(os.path.join(get_info.root_directory, 'build', 'libaudioverse.dll'), path)
	#copy additional directories
	for i in files.get('additional_directories', []):
		shutil.copytree(os.path.join(source_dir, i), os.path.join(dest_dir, i))

def make_bindings():
	for name, func in generators.iteritems():
		files = func(get_info.all_info)
		write_files(files, os.path.join(get_info.root_directory, 'bindings', name), os.path.join(get_info.root_directory, 'build', 'bindings', name))

from . import get_info
from . import python
import os.path
import os
import shutil
import copy
import platform

generators = {
'python' : (python.make_python, python.artifacts, python.release),
}

def write_files(files, source_dir, dest_dir):
    special_keys = {'dll_location', 'additional_directories', 'libsndfile_location', 'post_generate'}
    if os.path.exists(dest_dir):
        shutil.rmtree(dest_dir)
    for name, contents in files.items():
        if name in special_keys:
            continue
        fullpath = os.path.join(dest_dir, name)
        dir = os.path.split(fullpath)[0]
        if not os.path.exists(dir):
            os.makedirs(dir)
        with open(fullpath, 'wb') as f:
            f.write(contents)
    #handle the dll's copying.
    if platform.system() == 'Windows':
        dll_target = files.get('dll_location', '')
        path = os.path.join(dest_dir, dll_target)
        if not os.path.exists(path):
            os.makedirs(path)
        shutil.copy(os.path.join(get_info.get_root_directory(), 'build', 'libaudioverse.dll'), os.path.join(path, 'libaudioverse.dll'))
        #copy libsndfile
        path = os.path.join(dest_dir, files.get('libsndfile_location', ''))
        if not os.path.exists(path):
            os.makedirs(path)
        shutil.copy(os.path.join(get_info.get_root_directory(), "build", 'libsndfile-1.dll'), os.path.join(path, 'libsndfile-1.dll'))
    #copy additional directories
    for i in files.get('additional_directories', []):
        shutil.copytree(os.path.join(source_dir, i), os.path.join(dest_dir, i))

def make_bindings():
    if not os.path.exists(os.path.join(get_info.get_root_directory(), "build", "artifacts")):
        os.makedirs(os.path.join(get_info.get_root_directory(), "build", "artifacts"))
    all_info = get_info.get_all_info()
    artifacts = []
    for name, funcs in generators.items():
        generator, artifacts, release = funcs
        #we copy so that the generators can modify data as they need.
        files = generator(copy.deepcopy(all_info))
        source_dir = os.path.join(get_info.get_root_directory(), 'bindings', name)
        dest_dir = os.path.join(get_info.get_root_directory(), 'build', 'bindings', name)
        write_files(files, source_dir, dest_dir)
        #Call the post_generate hook, if any.
        #This is always called from the directory the bindings were put in.
        os.chdir(dest_dir)
        files.get('post_generate', lambda x: None)(dest_dir)
        for i in artifacts(dest_dir):
            name = os.path.split(i)[1]
            dest = os.path.join(get_info.get_root_directory(), "build", "artifacts", name)
            shutil.copy(i, os.path.join(get_info.get_root_directory(), "build", "artifacts", name))
        if get_info.is_release():
            print("Releasing", name)
            release(dest_dir)

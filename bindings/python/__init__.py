import jinja2
from collections import OrderedDict
import re
from .. import transformers, get_info, doc_helper
from . import doc_filters
import os
import os.path
import subprocess
import sys
import platform
import pypandoc
import glob
import distutils.util
from .releasing import *

ctypes_map = {
'int' : 'ctypes.c_int',
'unsigned int' : 'ctypes.c_uint',
'float' : 'ctypes.c_float',
'double' : 'ctypes.c_double',
'char': 'ctypes.c_char',
'void': 'None',
}

def ctypes_string(typeinfo, offset = 0, typedef_prefix = None):
    """Convert a type to a ctypes string.  Offset is used by the template _lav.py.t to make output argument strings and is subtracted from the levels of indirection."""
    global typedefs
    if offset != 0:
        assert typeinfo.indirection-offset >= 0
        return ctypes_string(get_info.TypeInfo(typeinfo.base, typeinfo.indirection-offset), 0, typedef_prefix)
    if typeinfo.indirection == 1 and typeinfo.base == 'void':
        return "ctypes.c_void_p"
    elif typeinfo.indirection == 1 and typeinfo.base == 'char':
        return "ctypes.c_char_p"
    elif typeinfo.indirection == 1 and isinstance(typeinfo.base, get_info.FunctionInfo):
        return ctypes_function_helper(typeinfo.base, typedef_prefix)
    elif typeinfo.indirection == 0 and typeinfo.base in typedefs:
        if typedef_prefix is None:
            return typeinfo.base
        else:
            return typedef_prefix + typeinfo.base
    elif typeinfo.indirection == 0:
        return ctypes_map[typeinfo.base]
    else:
        return "ctypes.POINTER(" + ctypes_string(get_info.TypeInfo(typeinfo.base, typeinfo.indirection-1), 0, typedef_prefix) + ")"

def ctypes_function_helper(func, typedef_prefix):
    retstr = ctypes_string(func.return_type, 0, typedef_prefix)
    argstr = ", ".join([retstr] + [ctypes_string(i.type, 0, typedef_prefix) for i in func.args])
    return "ctypes.CFUNCTYPE(" + argstr + ")"

def post_generate(dir):
    """Make a wheel and build docs if running in Windows."""
    if platform.system() == 'Windows':
        if os.getenv('APPVEYOR') is not None:
            command = [os.getenv("PYTHON_COMMAND")]
            print("In Appveyor. Using", command)
        else:
            command = ["py", "-3"]
            print("Not in Appveyor. Using", command)
        print("Building wheel and documentation for Python bindings.")
        sys.stdout.flush()
        subprocess.call(command + ["setup.py", "bdist_wheel", "--universal"], shell=True)
        subprocess.call(command + ["setup.py", "build_sphinx"], shell = True  )
        print("Attempting to rename wheel for Pypi upload.")
        #We know exactly where the wheel lives.
        path = r"dist\*any.whl"
        #Get its name.
        possibles=glob.glob(path)
        if len(possibles) == 0:
            raise ValueError("No wheels found. This should never happen.")
        if len(possibles) > 1:
            raise ValueError("Multiple wheels found. This should never happen.")
        #Otherwise, it's the first one.
        wh = possibles[0]
        #From pep 425
        platform_tag = distutils.util.get_platform().replace(".", "_").replace("-", "_")
        nwh = wh[:-len("any.whl")]+"{}.whl".format(platform_tag)
        #Okay. Rename it.
        os.rename(wh, nwh)
    else:
        print("Python bindings: Running on a non-windows platform. Skipping wheel and docs generation.")

def artifacts(dir):
    """List of artifacts. These files will be copied to an artifacts directory. Putting directories here is not allowed."""
    return [os.path.join(dir, i) for i in glob.glob("dist/*.whl")]

def make_python(info):
    #get our directory.
    source_dir = os.path.split(__file__)[0]
    #prepare our docs:
    doc_helper.prepare_docs(info,
    node=doc_filters.node, param=doc_filters.param, enum = doc_filters.enum,
    codelit=doc_filters.codelit, latex = doc_filters.latex)
    #we have to inject into the global namespace: the templates should not have to move typedef info around for us.
    global typedefs
    typedefs = info['typedefs']
    context = dict()
    context.update(info)
    context.update(transformers.get_jinja2_functions(info))
    context['ctypes_string_func'] = ctypes_string #ugly workaround for what appears to be a bug in jinja2.
    env = jinja2.Environment(loader = jinja2.PackageLoader(__package__, ""), undefined = jinja2.StrictUndefined, trim_blocks = True)
    env.filters.update(transformers.get_jinja2_filters(info))
    env.filters['ctypes_string'] = ctypes_string
    return {
        'libaudioverse/_lav.py' : env.get_template('libaudioverse/_lav.py.t').render(context).encode('utf8'),
        'libaudioverse/_libaudioverse.py' : env.get_template('libaudioverse/_libaudioverse.py.t').render(context).encode('utf8'),
        'libaudioverse/__init__.py': env.get_template('libaudioverse/__init__.py.t').render(context).encode('utf8'),
        'setup.py': env.get_template('setup.py.t').render(context).encode('utf8'),
        'setup.cfg': open(os.path.join(source_dir, 'setup.cfg'), 'rb').read(),
        'README.rst': pypandoc.convert(os.path.join(info['root_dir'], 'readme.md'), 'rst').encode("utf8"),
        'dll_location': 'libaudioverse',
        'libsndfile_location': 'libaudioverse',
        'additional_directories': [
            'examples',
            'docs',
        ],
        'post_generate': post_generate,
    }

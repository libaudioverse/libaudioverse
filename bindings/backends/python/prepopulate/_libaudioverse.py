{%import 'macros.t' as macros with context%}
import ctypes
import ctypes.util
import os.path
import os
import sys

#It is important that we don't use the platform module because it does not exist inside NVDA.
if sys.platform == 'win32':
    #this is a windows hack.
    #we want it to find out libsndfile before the system one in frozen executables, so we do this.
    #If it fails, we fall back to the system.
    #this latter point is what makes NVDA add-ons work right: they use the preloading a dll trick on Windows.
    if hasattr(sys, 'frozen'):
        try:
            path = os.path.join(os.path.abspath(os.path.dirname(sys.executable)), 'libaudioverse')
            libsndfile_module = ctypes.cdll.LoadLibrary(os.path.join(path, 'libsndfile-1.dll'))
            libaudioverse_module = ctypes.cdll.LoadLibrary(os.path.join(path, 'libaudioverse.dll'))
        except:
            libsndfile_module = ctypes.cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)), 'libsndfile-1.dll'))
            libaudioverse_module = ctypes.cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)), 'libaudioverse.dll'))
    else:
        libsndfile_module = ctypes.cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)), 'libsndfile-1.dll'))
        libaudioverse_module = ctypes.cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)), 'libaudioverse.dll'))
else:
    libaudioverse_name = ctypes.util.find_library("libaudioverse")
    try:
        libaudioverse_module = ctypes.cdll.LoadLibrary(libaudioverse_name)
    except:
      #Assume it's in /usr/local and try that.
      libaudioverse_module = ctypes.cdll.LoadLibrary("/usr/local/lib/"+libaudioverse_name)

{%for name, val in constants.items() -%}
{{name}} = {{val}}
{%endfor%}

{%for n, t in typedefs.items()%}
{{n}} = {{t|ctypes_string}}
{%endfor%}

{%for name, info in functions.items()-%}
{{name}} = ctypes.CFUNCTYPE({{info.return_type|ctypes_string}}
{%-if info.args|length > 0%}, {%endif%}{#some functions don't have arguments; if it doesn't, we must avoid the first comma#}
{%-for arg in info.args-%}
{{arg.type|ctypes_string}}
{%-if not loop.last%}, {%endif-%}{#put in a comma and space if needed#}
{%-endfor-%}
)(('{{name}}', libaudioverse_module))
{%endfor%}

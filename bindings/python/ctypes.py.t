import ctypes

{%-macro ctypes_string_helper(base, indirection)-%}
{%-if base == 'void' and indirection == 1%}ctypes.c_void_p
{%-elif base == 'char' and indirection == 1%}ctypes.c_char_p
{%-elif indirection > 0%}ctypes.POINTER({{ctypes_string_helper(base, indirection-1)}})
{%-elif indirection == 0%}ctypes.{{ctypes_map[base]}}
{%-endif%}
{%-endmacro%}

{%-macro ctypes_string(t)-%}
{{ctypes_string_helper(t.base, t.indirection)}}
{%-endmacro%}

libaudioverse_module = ctypes.cdll.LoadLibrary('libaudioverse.dll')

{%for name, val in constants.iteritems() -%}
{{name}} = {{val}}
{%endfor%}

{%for name, info in functions.iteritems()-%}
{{name}} = ctypes.CFUNCTYPE({{ctypes_string(info.return_type)}}
{%-if info.args|length > 0%}, {%endif%}{#some functions don't have arguments; if it doesn't, we must avoid the first comma#}
{%-for arg in info.args-%}
{{ctypes_string(arg.type)}}
{%-if not loop.last%}, {%endif-%}{#put in a comma and space if needed#}
{%-endfor-%}
)(('{{name}}', libaudioverse_module))
{%endfor%}

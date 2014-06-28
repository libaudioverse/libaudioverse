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

{%-macro destination_string(t)-%}
{{ctypes_string_helper(t.base, t.indirection-1)}}
{%-endmacro-%}

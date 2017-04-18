import bindings.metadata_description as md

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
    if offset != 0:
        assert typeinfo.indirection-offset >= 0
        return ctypes_string(md.TypeInfo(typeinfo.base, typeinfo.indirection-offset), 0, typedef_prefix)
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
        return "ctypes.POINTER(" + ctypes_string(md.TypeInfo(typeinfo.base, typeinfo.indirection-1), 0, typedef_prefix) + ")"

def ctypes_function_helper(func, typedef_prefix):
    retstr = ctypes_string(func.return_type, 0, typedef_prefix)
    argstr = ", ".join([retstr] + [ctypes_string(i.type, 0, typedef_prefix) for i in func.args])
    return "ctypes.CFUNCTYPE(" + argstr + ")"

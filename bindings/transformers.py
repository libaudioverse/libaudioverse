"""Implements standard transformers: Lav_ remover, camelcase-to-underscore and back, etc.

These are then made available to templates."""
import re
import os.path
from . import get_info

def without_lav(s):
    """Remove the lav_ prefix from a Libaudioverse identifier."""
    if s.startswith('Lav_'):
        return s[4:]
    else:
        return s

def underscores_to_camelcase(s, capitalize_first = False):
    """Convert an identifer of the form ab_cd_ef_gh to abCdEfGh.  If capitalize_first is True, convert to AbCdEfGh."""
    what = s.lower()
    what = re.sub('_([a-z])', lambda x: x.group(1).upper(), what)
    if capitalize_first:
        what = what[0].upper() + what[1:]
    return what

def camelcase_to_underscores(s):
    """Converts camelcase identifiers to have underscores and be all lowercase."""
    what = s[0].lower()+s[1:]
    what =  re.sub('[A-Z]', lambda x: '_' + x.group(0).lower(), what)
    return what

def prefix_filter(l, prefix):
    """Expects l, an iterable of strings, and a prefix.  Returns a list consisting of all strings in l that begin with prefix."""
    return filter(lambda x: x.startswith(prefix), l)

def regexp_filter(l, regexp):
    return filter(lambda x: re.match(regexp, x) is not None, l)


def remove_filter(l, item):
    """Returns l without all instances of item."""
    return filter(lambda x: x != item, l)

def strip_prefix(s, prefix):
    if s.startswith(prefix):
        return s[len(prefix):]
    else:
        return s

def strip_suffix(s, suffix):
    """Strips a suffix, if that suffix exists."""
    if s.endswith(suffix):
        return s[0:-len(suffix)]
    else:
        return s

def common_prefix(l):
    return os.path.commonprefix(l)

#infrastructure for printing C function and type prototypes, etc.

def type_to_string(type, so_far= None, indirection =0, typedefs =  None, apply_typedefs=False):
    """Turns a type into a string that matches C syntax."""
    #Special case of pure typedef, which is the case in which we are most interested in expanding:
    if apply_typedefs and type.base in typedefs and type.indirection == 0:
        return type_to_string(typedefs[type.base])
    #Special case of function pointers:
    if isinstance(type.base, get_info.FunctionInfo) and type.indirection == 1:
        return function_pointer_to_string(type.base)
    #first, the level 0 qualifiers.
    q= type.quals.get(indirection, [])
    if len(q):
        qualstring=" ".join(q)+" "
    else:
        qualstring=""
    if so_far is None:
        return type_to_string(type, qualstring+type.base, type.indirection)
    if indirection == 0: #We've come full circle and/or had no indirection to start with.
        return so_far
    #append a * and the qualstring.
    so_far+="*"
    if len(qualstring):
        so_far+=" "+qualstring
    return type_to_string(type, so_far, indirection-1)

def function_pointer_to_string(f):
    """Converts a function pointer to a string."""
    return function_to_string(f, True)

def function_to_string(f, is_pointer = False):
    ret_string = type_to_string(f.return_type)+" "
    if is_pointer:
        name_string = "("+f.name+"*)"
    else:
        name_string=f.name
    arg_strings = []
    for i in f.args:
        s=type_to_string(i.type)+" "+i.name
        arg_strings.append(s)
    arg_string="("+", ".join(arg_strings)+")"
    return ret_string+name_string+arg_string

def compute_involved_typedefs(f, all_info):
    """Computes a list of all typedefs involved in a functions' definition."""
    retval = []
    for i in [f.return_type]+[q.type for q in f.args]:
        if i.base == 'LavError' or i.base == 'LavHandle':
            continue
        if i.base in all_info['typedefs']:
            retval.append(i.base)
    retval.sort()
    for i, j in enumerate(retval):
        retval[i] = j, all_info['typedefs'][j]
    return retval

def get_jinja2_filters(all_info):
    return {'without_lav': without_lav,
        'camelcase_to_underscores': camelcase_to_underscores,
        'underscores_to_camelcase': underscores_to_camelcase,
        'remove_filter': remove_filter,
        'prefix_filter': prefix_filter,
        'regexp_filter': regexp_filter,
        'strip_prefix': strip_prefix,
        'strip_suffix': strip_suffix,
        'compute_involved_typedefs': lambda x: compute_involved_typedefs(x, all_info),
        'function_to_string': function_to_string,
        'function_pointer_to_string': function_pointer_to_string,
        'type_to_string': type_to_string,
    }

def get_jinja2_functions(all_info):
    return {
        'common_prefix': common_prefix,
    }

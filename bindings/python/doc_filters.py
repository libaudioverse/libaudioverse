from .. import transformers

def node(all_info, s):
    return ":class:`{}`".format(transformers.underscores_to_camelcase(transformers.without_lav(s)[len("OBJTYPE_"):], True))

def param(all_info, s):
    s=transformers.camelcase_to_underscores(s)
    if s.endswith("_handle"):
        s=s[:-len("_handle")]
    return "*{}*".format(s)

def enum(all_info, s):
    return ":class:`{}`".format(transformers.underscores_to_camelcase(transformers.without_lav(s), True))

#skip function, as Python doesn't expose them.

def codelit(all_info, s):
    return "``{}``".format(s)

def latex(all_info, s):
    return ":math:`{}`".format(s)
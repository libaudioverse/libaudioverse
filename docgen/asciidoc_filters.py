def link(id, text):
    return "<<{},{}>>".format(id, text)

def enum(all_info, x):
    if x in all_info['metadata']['enumerations']:
        return link("enum-"+x, x)
    else:
        return "`{}`".format(x) #Because it isn't documented.

def node(all_info, x):
    #reference a node.
    if x not in all_info['metadata']['nodes']:
        raise ValueError("{} is not a valid node".format(x))
    link_text = all_info['metadata']['nodes'][x]['doc_name'] + " node"
    link_id = "node-{}".format(x)
    return link(link_id, link_text)

def param(all_info, x):
    #This is identity.
    return x

def codelit(all_info, x):
    return "`{}`".format(x)

def latex(all_info, s):
    return "stem:[{}]".format(s)

def bound_filters(info):
    return {
        'enum': lambda x: enum(info, x),
        'node': lambda x: node(info, x),
        'param': lambda x: param(info, x),
        'codelit': lambda x: codelit(info, x),
        'latex': lambda x: latex(info, x),
    }

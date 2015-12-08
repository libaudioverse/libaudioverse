import jinja2

def identity(all_info, x):
    return x

def prepare_docs(all_info,
param = identity, node = identity, enum = identity, function = identity,
codelit=identity, extra_function = identity,
latex = identity, property = identity,
no_doc_description = "No description defined."):
    """Runs over the all_info specified.
Renders all documentation therein through Jinja2, using the filters passed to this function.
Note that no globals are provided and the default for all arguments is to do nothing to the text.
Furthermore, some "missing" fields such as doc_description are added by this function.  Do all desired error checking first.

The filters must accept two arguments.  The first must be an all_info dict.  This dict may be partially modified by this function.
The second is the string to transform."""
    env=jinja2.Environment(undefined=jinja2.StrictUndefined)
    env.filters['param'] = lambda x: param(all_info, x)
    env.filters['node'] = lambda x: node(all_info, x)
    env.filters['enum'] = lambda x: enum(all_info, x)
    env.filters['function'] = lambda x: function(all_info, x)
    env.filters['extra_function'] = lambda x: extra_function(all_info, x)
    env.filters['codelit'] = lambda x: codelit(all_info, x)
    env.filters['latex'] = lambda x: latex(all_info, x)
    env.filters['property'] = lambda x: property(all_info, x)
    #yes, this is a local function.
    def render(s):
        template=env.from_string(s)
        res = template.render()
        return res
    #This one is, as well. It lets us capture the Jinja2 environment:
    def prepare_function(func):
        func['doc_description'] = render(func.get('doc_description', no_doc_description))
        if 'params' in func: #some functions don't have params
            for n, p in func['params'].items():
                func['params'][n] = render(p)
                
    for name, func in all_info['metadata']['functions'].items():
        prepare_function(func)

    for name, d in all_info['metadata']['nodes'].items():
        d['doc_description'] = render(d.get('doc_description', no_doc_description))
        for prop in d.get('properties', dict()).values():
            prop['doc_description'] = render(prop.get('doc_description', ""))
        for name, func in d.get('extra_functions', dict()).items():
            prepare_function(func)
        #callbacks are function-like enough:
        for name, cb in d.get('callbacks', dict()).items():
            prepare_function(cb)
    #Run over the enums and render them:
    for enum in all_info['metadata']['enumerations'].values():
        enum['doc_description']=render(enum['doc_description'])
        for k, v in enum['members'].items():
            enum['members'][k] = render(v)
    #no return, we modify in place.

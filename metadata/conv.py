import jinja2
import yaml
import os.path
here = os.path.abspath(os.path.split(__file__)[0])


function_template = jinja2.Template('''
c_function(
    name = "{{name}}",
    category = "{{category}}",
    doc = r"""{{doc}}""",
    {%if params|length%}param_docs = {
        {%for i, j in params.items()%}"{{i}}": r"""{{j}}""",{%endfor%}
    }{%endif-%}
)
''')


with open(os.path.join(here, "out.py"), "w") as out:
    with open(os.path.join(here, "function_categories.y")) as f:
        function_categories = yaml.load(f.read())['function_categories']
    with open(os.path.join(here, "functions.y")) as f:
        functions = yaml.load(f.read())['functions']
    for i in function_categories:
        catname = i['name']
        doc_name = i['doc_name']
        doc_description = i['doc_description']
        out.write('register_c_category(name = "{}", doc_name = "{}", doc_description = r"""{}""")'.format(catname, doc_name, doc_description))
        out.write("\n")
        wanted_functions = [k[0] for k in functions.items() if k[1]['category'] == catname]
        wanted_functions.sort()
        for name in wanted_functions:
            j = functions[name]
            if j['category'] != catname:
                continue
            doc = j['doc_description']
            params = j.get('params', dict())
            category = j['category']
            tmp = function_template.render(name = name, category = category, doc = doc, params = params)
            out.write(tmp)
            out.write("\n")

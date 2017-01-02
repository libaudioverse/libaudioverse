#we have to have the root dir of the repository in sys.path.
import os.path
import sys
import jinja2
import inspect
sys.path = [os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]] + sys.path
import bindings.dsl as dsl


builder = dsl.Builder()
globals = {i[0]: getattr(builder, i[0]) for i in inspect.getmembers(builder) if not i[0].startswith("_")}
p = sys.argv[1]
print(globals)
with open(p) as f:
    src = f.read()
    exec(src, globals)

print(len(builder.categories))
print(len(builder.functions))

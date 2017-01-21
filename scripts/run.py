#we have to have the root dir of the repository in sys.path.
import os
import os.path
import sys
import jinja2
import inspect
sys.path = [os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]] + sys.path
import bindings.dsl as dsl
import bindings.get_info as get_info
import glob

root = get_info.get_root_directory()
metadata = os.path.join(root, "metadata")

info = get_info.get_all_info()



builder = dsl.Builder(info)
globals = {i[0]: getattr(builder, i[0]) for i in inspect.getmembers(builder) if not i[0].startswith("_")}
globals.update({i[0]: getattr(dsl, i[0]) for i in inspect.getmembers(dsl) if not i[0].startswith("_") and i[0] != "Builder"})

for dir in os.walk(metadata):
    for file in dir[2]:
        path = os.path.join(dir[0], file)
        if not path.endswith(".py"):
            continue
        print("Running ", path)
        with open(path) as f:
            src = f.read()
            exec(src, globals)

print(len(builder.categories))
print(len(builder.functions))

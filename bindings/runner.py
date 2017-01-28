"""This is the main entry point for the bindings generator.

The launching script imports this file and calls run."""
import os, os.path
import copy
import inspect
import shutil
from . import get_info, dsl
import bindings.backends.python

root_directory = get_info.get_root_directory()
metadata_directory = os.path.join(root_directory, "metadata")

backend_list = [
    ("python", bindings.backends.python),
]

def _run_backend(metadata, backend, destination, dry, build):
    path = os.path.split(os.path.abspath(backend.__file__))[0]
    metadata = copy.deepcopy(metadata) # We will change this in a minute by rendering docstrings.
    backend = backend.Backend(backend_path = path)
    # Todo: render the documentation strings, with as-yet-unwritten helper methods in metadata_description.
    enums = list(metadata.enums.keys())
    enums.sort()
    for e in enums:
        backend.visit_enum(metadata.enums[e])
    for n, t in metadata.typedefs.items():
        backend.visit_typedef(n, t)
    for m in metadata.enums["Lav_ERRORS"].members:
        backend.visit_error_enum(m)
    functions = list(metadata.functions.keys())
    functions.sort()
    for n in functions:
        backend.visit_c_function(metadata.functions[n])
    nodes = list(metadata.nodes.keys())
    nodes.sort()
    for n in nodes:
        n = metadata.nodes[n]
        backend.begin_node(n)
        if len(n.properties):
            backend.begin_properties(n)
            for p in n.properties:
                backend.visit_property(n, p)
            backend.end_properties(n)
        if len(n.callbacks):
            backend.begin_callbacks(n)
            for c in n.callbacks:
                backend.visit_callback(n, c)
            backend.end_callbacks(n)
        if len(n.extra_functions):
            backend.begin_extra_functions(n)
            for f in n.extra_functions:
                backend.visit_extra_function(n, f)
            backend.end_extra_functions(n)
        backend.end_node(n)
    if not dry:
        # We need to compile and actually write the files.
        for path, lines in backend._output_files_text.items():
            path = os.path.join(destination, path)
            directory = os.path.split(path)[0]
            if not os.path.exists(directory):
                os.makedirs(directory)
            if backend.suppress_whitespace_lines:
                lines = [("" if i.isspace() else i) for i in lines]
            with open(path, "w", encoding = backend.encoding) as f:
                for line in lines:
                    f.write(line + "\n")
        for path, data in backend._output_files_binary.items():
            path = os.path.join(destination, path)
            directory = os.path.split(path)[0]
            if not os.path.exists(directory):
                os.makedirs(directory)
            with open(path, "wb") as f:
                f.write(data)
    if build:
        artifacts = backend.build()
        # Todo: something with the artifacts, but I haven't decided on what yet.

def run(dry = False, release = False):
    print("Loading and compiling metadata.")
    globals = dict()
    c_info = get_info.get_all_info()
    builder = dsl.Builder(c_info = c_info)
    globals.update({i[0]: i[1] for i in inspect.getmembers(dsl) if not i[0].startswith("_")})
    globals.update({i[0]: getattr(builder, i[0]) for i in inspect.getmembers(builder) if not i[0].startswith("_")})
    for (dirpath, dirnames, filenames) in os.walk(metadata_directory):
        for p in [os.path.join(dirpath, i) for i in filenames]:
            if not p.endswith(".py"):
                continue
            with open(p) as f:
                src = f.read()
                # Just exec is insufficient because we want tracebacks to be right.
                src = compile(src, p, "exec")
                exec(src, globals)
    metadata = builder.finish()
    print("Running bindings backends.")
    if dry:
        print("Dry run.  Files will not be written.")
    else:
        if os.path.exists(os.path.join(root_directory, "build", "bindings")):
            shutil.rmtree(os.path.join(root_directory, "build", "bindings"))
    for name, backend in backend_list:
        destination = os.path.join(root_directory, "build", "bindings", name)
        _run_backend(metadata, backend, destination = destination, dry = dry, build = not dry)

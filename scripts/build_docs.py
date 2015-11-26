#we have to have the root dir of the repository in sys.path.
import os.path
import sys
import shutil
import subprocess

repository_root = os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]
sys.path = [repository_root] + sys.path
import docgen
import bindings.get_info

if __name__ == '__main__':
    print("Building documentation...")
    info=bindings.get_info.get_all_info()
    docgen.prepare_docs(info)
    dest_dir = os.path.join(repository_root, 'build', 'documentation')

    node_reference = docgen.make_node_reference(info)
    c_api_docs=docgen.make_c_api(info)
    enum_docs=docgen.make_enumerations(info)
    if os.path.exists(dest_dir):
        shutil.rmtree(dest_dir)

    shutil.copytree(os.path.join(repository_root, 'documentation'), dest_dir)

    with open(os.path.join(dest_dir, 'node_reference.asciidoc'), 'w') as f:
        f.write(node_reference)
    with open(os.path.join(dest_dir, 'c_api.asciidoc'), 'w') as f:
        f.write(c_api_docs)
    with open(os.path.join(dest_dir, 'enumerations.asciidoc'), 'w') as f:
        f.write(enum_docs)

    subprocess.call(["asciidoctor", os.path.join(dest_dir, 'libaudioverse_manual.asciidoc')], stderr= subprocess.STDOUT, shell = True)

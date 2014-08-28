#we have to have the root dir of the repository in sys.path.
import os.path
import sys
import shutil
repository_root = os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]
sys.path = [repository_root] + sys.path
import docgen

print "Building documentation..."
dest_dir = os.path.join(repository_root, 'documentation')

property_table = docgen.make_property_table()

with file(os.path.join(dest_dir, 'object_reference.asciidoc'), 'wb') as f:
	f.write(property_table)

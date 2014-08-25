import os.path
import os
import sys
repository_root = os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]
sys.path = [repository_root] + sys.path
from bindings import get_info
import jinja2

def make_property_table():
	pass

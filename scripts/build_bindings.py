#we have to have the root dir of the repository in sys.path.
import os.path
import sys
sys.path = [os.path.split(os.path.split(__file__)[0])[0]] + sys.path
import bindings.make_bindings


if __name__ == '__main__':
	print "Building bindings..."
	bindings.make_bindings.make_bindings()

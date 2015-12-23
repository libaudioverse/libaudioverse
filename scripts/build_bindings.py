#we have to have the root dir of the repository in sys.path.
import os.path
import sys
import jinja2
sys.path = [os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]] + sys.path
import bindings.make_bindings


if __name__ == '__main__':
    print("Building bindings...")
    try:
        bindings.make_bindings.make_bindings()
    except jinja2.exceptions.TemplateSyntaxError as e:
        print("Error in template: {} line {}: {}".format(e.name, e.lineno, e.message))
        sys.exit(1)
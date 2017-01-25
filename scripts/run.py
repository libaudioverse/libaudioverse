#we have to have the root dir of the repository in sys.path.

import os.path
import sys
sys.path = [os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]] + sys.path

needed = sys.argv[1]
sys.argv = [sys.argv[0]]+sys.argv[2:]

path = os.path.join(
    os.path.split(os.path.abspath(__file__))[0],
    needed+".py"
)

with open(path) as f:
    source = f.read()

exec(source)

import os
import os.path
import glob
import subprocess
import sys
import re
from .. import get_info

def release(dir):
    version = get_info.get_version()
    print("Release version is: ", version)
    if get_info.is_test_release():
        pypi_repo = "pypitest"
    else:
        pypi_repo = "pypi"
    #Call twine.
    if os.getenv("PYPI_PASSWORD") is not None:
        print("Found Pypi password.  Uploading to Pypi.")
        subprocess.check_output(" ".join([r"c:\python35\python", "-m", "twine", "upload",
        "--config-file", os.path.join(get_info.get_root_directory(), "pypirc.cfg"),
        "-r", pypi_repo,
        "-u", "camlorn",
        "-p", os.getenv("PYPI_PASSWORD"),
        os.path.join(dir, "dest", "*.whl")]), shell=True, stdin = sys.stdin, stdout = sys.stdout, stderr = sys.stderr)

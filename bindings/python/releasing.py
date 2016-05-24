import os
import os.path
import glob
import subprocess
import sys
import re

def do_appveyor_release():
    tag = os.getenv("APPVEYOR_REPO_TAG_NAME")
    if not tag:
        tag = "" #hack to avoid repeating a message.
    match = re.match("(test)*release-(.+)", tag)
    if not match:
        print("Not a release. Skipping release processing.")
        sys.exit(0)
    version = match.group(2)
    is_test = match.group(1)
    #Convert to a boolean for clarity.
    if is_test:
        is_test = True
    else:
        is_test = False
    print("Release version is: ", version)
    if is_test:
        print("This is a test release.")
    if is_test:
        pypi_repo = "pypitest"
    else:
        pypi_repo = "pypi"
#Call twine.
if os.getenv("PYPI_PASSWORD") is not None:
    print("Found Pypi password.  Uploading to Pypi.")
    subprocess.check_output(" ".join([r"c:\python35\python", "-m", "twine", "upload",
    "--config-file", "pypirc.cfg",
    "-r", pypi_repo,
    "-u", "camlorn",
    "-p", os.getenv("PYPI_PASSWORD"),
    nwh]), shell=True)
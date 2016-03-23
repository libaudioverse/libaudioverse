import os
import os.path
import glob
import subprocess

tag = os.getenv("APPVEYOR_REPO_TAG_NAME")
if not tag or not tag.startswith("release-"):
    print("Not a release.")
    sys.exit(0)

print("Attempting to rename wheel for Pypi upload.")
#We know exactly where the wheel lives.
path = r"c:\projects\libaudioverse\build\bindings\python\dist\*any.whl"
#Get its name.
possibles=glob.glob(path)
if len(possibles) == 0:
    raise ValueError("No wheels found. Cannot upload to Pypi.  Aborting CI.")
if len(possibles) > 1:
    raise ValueError("Multiple wheels found. Cannot upload to Pypi. Aborting CI.")
#Otherwise, it's the first one.
wh = possibles[0]
nwh = wh[:-len("any.whl")]+"win32.whl"
#Okay. Rename it.
os.rename(wh, nwh)
#Call twine.
if os.getenv("PYPI_PASSWORD") is not None:
    print("Found Pypi password.  Uploading to Pypi.")
    subprocess.check_output(" ".join([r"c:\python35\python", "-m", "twine", "upload",
    "--config-file", "pypirc.cfg",
    "-u", "camlorn",
    "-p", os.getenv("PYPI_PASSWORD"),
    nwh]), shell=True)
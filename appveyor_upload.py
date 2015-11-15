import paramiko
import os
import os.path
import glob
import subprocess

username = os.getenv("DEPLOYMENT_USERNAME")
password = os.getenv("DEPLOYMENT_PASSWORD")
host = os.getenv("DEPLOYMENT_HOST")
port = 22

if host is None:
    print("Host?")
    host = input()
if username is None:
    print("Username?")
    username = input()
if password is None:
    print("Password?")
    password = input()

print("Uploading to camlorn.net.")
transport=paramiko.Transport(host, 22)
transport.connect(username = username, password = password)
with paramiko.SFTPClient.from_transport(transport) as sftp:
    script_dir = os.path.split(os.path.abspath(__file__))[0]
    build_dir = os.path.join(script_dir, "build")
    sftp.chdir("releases/libaudioverse")
    sftp.put(
    os.path.join(build_dir, "libaudioverse_master.zip"),
    "libaudioverse_master.zip")
    
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
    subprocess.call(["py", "-3", "-m", "twine", "upload",
    "--config-file", "pypirc.cfg",
    "-u", "camlorn",
    "-p", os.getenv("PYPI_PASSWORD"),
    nwh], shell=True)
import paramiko
import os
import os.path

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

transport=paramiko.Transport(host, 22)
transport.connect(username = username, password = password)
with paramiko.SFTPClient.from_transport(transport) as sftp:
    script_dir = os.path.split(os.path.abspath(__file__))[0]
    build_dir = os.path.join(script_dir, "build")
    sftp.chdir("releases/libaudioverse")
    sftp.put(
    os.path.join(build_dir, "libaudioverse_master.zip"),
    "libaudioverse_master.zip")
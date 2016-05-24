import os.path
import distutils.util
import zipfile
import platform

if platform.system() != "Windows":
    print("Not Windows. Cannot currently generate zip.")
    sys.exit(0)


root_dir = os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]

artifacts_dir = os.path.join(root_dir, "build", "artifacts")

headers = [os.path.join(root_dir, "include", "libaudioverse", i) for i in
['libaudioverse.h', 'libaudioverse_properties.h', 'libaudioverse3d.h']]

libfiles = [os.path.join(root_dir, "build", i) for i in
['libaudioverse.lib', 'libaudioverse.dll', 'libsndfile-1.dll']]

platform = distutils.util.get_platform().replace("-", "_")

zipname = "libaudioverse_{}.zip".format(platform)

print("Making zip:", zipname)

if not os.path.exists(os.path.join(root_dir, "build", "artifacts")):
    os.makedirs(os.path.join(root_dir, "build", "artifacts"))

zip = zipfile.ZipFile(os.path.join(root_dir, "build", "artifacts", zipname), "w")

for i in headers:
    hname = os.path.split(i)[1]
    zip.write(i, os.path.join("include", "libaudioverse", hname))

for i in libfiles:
    lname = os.path.split(i)[1]
    zip.write(i, os.path.join("lib", lname))

zip.close()

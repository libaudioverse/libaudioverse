#Push docs from appveyor to GitHub pages.

import os
import os.path
import subprocess
import sys
import shutil

root = os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]

os.chdir(root)

try:
    subprocess.check_output(['git', 'clone', 'https://github.com/camlorn/libaudioverse', '-b', 'gh-pages', '--single-branch', 'libaudioverse_docs'],
    stderr = subprocess.STDOUT, shell = True)
    os.chdir("libaudioverse_docs")
    if not os.path.exists("docs"):
        os.makedirs("docs")
    os.chdir("docs")
    branch = os.getenv("APPVEYOR_REPO_BRANCH")
    if branch is None or len(branch) == 0:
        print("Couldn'tg et branch.")
        sys.exit(1)
    if not os.path.exists("branches"):
        os.makedirs("branches")
    os.chdir("branches")
    if os.path.exists(branch):
        shutil.rmdir(branch)
    os.makedirs(branch)
    os.chdir(branch)
    #Okay, and copy:
    bindings_dir = os.path.join(root, "build", "bindings")
    manual_dir = os.path.join(root, "build", "documentation")
    shutil.copy(os.path.join(manual_dir, "libaudioverse_manual.html"), "libaudioverse_manual.html")
    shutil.copytree(os.path.join(bindings_dir, "python", "docs", "_build", "html"),
    "python")
    #Finally, commit.
    os.chdir(os.path.join(root, "libaudioverse_docs"))
    subprocess.check_output(['git', 'add', '-A'], stderr = subprocess.STDOUT, shell = True)
    subprocess.check_output(['git', 'commit', '-m',
    "Update manual from Appveyor build {}".format(os.getenv("APPVEYOR_BUILD_NUMBER"))],
    shell = True, stderr = subprocess.STDOUT)
    subprocess.check_output(["git", "push"], shell = True, stderr = subprocess.STDOUT)
except subprocess.CalledProcessError as e:
    print("Couldn't complete documentation deploy due to error calling a command:")
    print(e.cmd)
    print("Output is:")
    print(e.output.decode("utf8"))

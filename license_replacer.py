#This file is kept in case I ever need to replace or revise the license headers again.
import glob
old_header = open("old_header.txt").read()
new_header = open("new_header.txt").read()
files = list(glob.iglob("src/**/*.cpp", recursive = True))+list(glob.iglob("include/**/*.hpp", recursive = True))+list(glob.iglob("include/**/*.h", recursive = True))
unhandled = []

for name in files:
    with open(name) as f:
        text = f.read()
        newtext = text.replace(old_header, new_header)
        if newtext == text:
            unhandled.append(name)
    with open(name, "w") as f:
        f.write(newtext)

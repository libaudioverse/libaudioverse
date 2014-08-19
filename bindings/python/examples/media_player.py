#a simple command line media player.
import libaudioverse
import os.path

dev = libaudioverse.Device(physical_output_index = -1)
print """Command line Media player.
Please enter the path to a file in a format supported by Libsndfile: typically wave or ogg.
Mono or stereo files will work fine.  Surround sound files will have additional channels stripped, keeping only the first two."""
filepath = raw_input()
filepath = os.path.abspath(filepath)
filenode = libaudioverse.FileObject(filepath)

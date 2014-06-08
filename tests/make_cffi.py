import subprocess
import cffi
import sys

input_file = 'include/libaudioverse/private_all.h'

if sys.platform == 'win32':
	command = 'cl /EP /DIS_TESTING ' + input_file
else:
	command  = 'cpp -D -D IS_TESTING ' + input_file

src = subprocess.check_output(command)
src = src.replace("\r\n", "\n")
src = src.replace("#pragma once", "")
src = src.replace("__declspec(dllexport)", "")
ffi = cffi.FFI()
ffi.cdef(src)

lav = ffi.dlopen("build/libaudioverse.dll")

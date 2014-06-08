import subprocess
import cffi
import sys

input_file = 'include/libaudioverse/private_all.h'

if sys.platform == 'win32':
	command = 'cl'
	args = '/EP /DIS_TESTING ' + input_file
	lib = 'libaudioverse.dll'
else:
	command  = 'cpp'
	args = '-D IS_TESTING ' + input_file
	lib = 'liblibaudioverse.so'

src = subprocess.check_output(command + ' ' + args, shell = True)
src = src.replace("\r\n", "\n")
src = src.replace("#pragma once", "")
src = src.replace("__declspec(dllexport)", "")
ffi = cffi.FFI()
ffi.cdef(src)

lav = ffi.dlopen("build/" + lib)

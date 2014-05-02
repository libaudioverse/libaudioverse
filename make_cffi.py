import subprocess
import cffi

input_file = 'include/libaudioverse/cffi.helper.h'

src = subprocess.check_output('cl /EP ' + input_file)
src = src.replace("\r\n", "\n")
src = src.replace("#pragma once", "")
src = src.replace("__declspec(dllexport)", "")
ffi = cffi.FFI()
ffi.cdef(src)

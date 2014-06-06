from make_cffi import *
from .polar import * #the helper function for unit vectors in specific directions.
from .conf import *

def setMatrix(mat, l):
	for i, j in enumerate(l):
		mat[i%4][i/4] = j

def test_transform_identity():
	trans = ffi.new("LavTransform")
	lav.identityTransform(trans)
	vec = ffi.new("LavVector")
	vec[0], vec[1], vec[2] = 5, 10, 20
	out = ffi.new("LavVector")
	lav.transformApply(trans, vec, out)
	for i, j in zip(vec, out):
		assert abs(i-j) < accuracy

def test_transform_invert_orthoganal():
	trans = ffi.new("LavTransform[4]")
	inv = ffi.new("LavTransform[4]")
	#we need a source of known orthoganal matrices.
	#we thus use the camera function.
	#it'll work so long as it returns something orthoganal, even if it fails its test.
	lav.cameraTransform(direct(0, -30), direct(0, 60), [5, -30, 0, 1], trans[0])
	lav.cameraTransform(direct(45, 0), direct(45, 90), [0, 0, 0, 1], trans[1],  )
	lav.cameraTransform(direct(0, -0), direct(0, -90), [0, 0, 0, 1], trans[2])
	lav.identityTransform(trans[3])
	trans[3][0][3] = 5
	trans[3][1][3] = 22
	trans[3][2][3] = -13
	for i, j in enumerate(trans):
		lav.transformInvertOrthoganal(j, inv[i])
	for ind, i, j in zip(xrange(len(trans)), trans, inv):
		input = ffi.new("LavVector", [0.5, 0.5, 0.5, 1])
		output = ffi.new("LavVector", [0, 0, 0, 1])
		lav.transformApply(i, input, output)
		lav.transformApply(j, output, output)
		for k, l in zip(input, output):
			assert abs(k-l) < accuracy

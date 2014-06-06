/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_all.h>
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**This is a matrix transformation library adequate for Libaudioverse's purposes**/

Lav_PUBLIC_FUNCTION void identityTransform(LavTransform t) {
	t[0][0] = 1.0f;
	t[1][1] = 1.0f;
	t[2][2] = 1.0f;
	t[3][3]=1.0f;
}

Lav_PUBLIC_FUNCTION void transformSetTranslation(LavTransform t, LavVector v) {
	t[0][3]=v[0];
	t[1][3]=v[1];
	t[2][3]=v[2];
}

Lav_PUBLIC_FUNCTION void transformGetTranslation(LavTransform t, LavVector out) {
	v[0]=m[0][3];
	v[1]=m[1][3];
	v[2]=m[2][3];
	v[3]=m[3][3];
}

Lav_PUBLIC_FUNCTION void transformApply(LavTransform t, LavVector in, LavVector out) {
	for(unsigned int i = 0; i < 4; i++) {
		out[i] = t[i][0]*in[0]+t[i][1]*in[1]+t[i][2]*in[2]+t[i][3]*in[3];
	}
}

Lav_PUBLIC_FUNCTION void transformInvertOrthoganal(LavTransform t) {
	for(int i = 0; i < 3; i++) {
		for(int j = 0; j < 3; j++) {
			t[i][j]=t[j][i];
		}
	}
	t[0][3]*=-1;
	t[1][3] *= -1;
	t[2][3] *= -1;
}

Lav_PUBLIC_FUNCTION float vectorDotProduct(LavVector a, LavVector b) {
	return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

Lav_PUBLIC_FUNCTION void vectorCrossProduct(LavVector a, LavVector b, LavVector out) {
	out[0] = a[1]*b[2]-a[2]*b[1];
	out[1]=a[2]*b[0]-a[0]*b[2];
	out[2] = a[0]*b[1]-a[1]*b[0];
}

Lav_PUBLIC_FUNCTION void cameraTransform(LavTransform t, LavVector at, LavVector up) {
	identityTransform(t);
	for(unsigned int i = 0; i < 3; i++) {
		t[0][i]=at[i];
		t[1][i]=up[i];
	}
	LavVector right;
	vectorCrossProduct(at, up, right);
	t[2][0]=right[0];
	t[2][1] = right[1];
	t[2][2] = right[2];
	transformInvertOrthoganal(t);
}

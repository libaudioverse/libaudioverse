/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_all.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

/**This is a matrix transformation library adequate for Libaudioverse's purposes**/

Lav_PUBLIC_FUNCTION void identityTransform(LavTransform out) {
	memset(out, 0, sizeof(LavTransform));
	out[0][0] = 1.0f;
	out[1][1] = 1.0f;
	out[2][2] = 1.0f;
	out[3][3]=1.0f;
}

Lav_PUBLIC_FUNCTION void transformApply(LavTransform trans, LavVector in, LavVector out) {
	LavVector tmp = {0};
	//we're multiplying a 1x3 by a 3x3. Output is therefore 1x3.
	for(int row = 0; row < 4; row++) {
		tmp[row] = in[0]*trans[row][0]+in[1]*trans[row][1]+in[2]*trans[row][2]+in[3]*trans[row][3];
	}
	memcpy(out, tmp, sizeof(LavVector));
}

Lav_PUBLIC_FUNCTION void transformMultiply(LavTransform t1, LavTransform t2, LavTransform out) {
	LavTransform tmp;
	memset(tmp, 0, sizeof(LavTransform));
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			for(int k = 0; k < 4; k++) {	
				tmp[i][j] += t1[i][k]*t2[k][j];
			}
		}
	}
	memcpy(out, tmp, sizeof(LavTransform));
}

Lav_PUBLIC_FUNCTION void transformTranspose(LavTransform t, LavTransform out) {
	LavTransform tmp = {0};
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			tmp[i][j] = t[j][i];
		}
	}
	memcpy(out, tmp, sizeof(LavTransform));
}

Lav_PUBLIC_FUNCTION void transformInvertOrthoganal(LavTransform t, LavTransform out) {
	LavTransform tmp = {0};
	LavVector trans = {0};
	trans[0] = -1*t[0][3];
	trans[1] = -1*t[1][3];
	trans[2] = -1*t[2][3];
	trans[3] = 0.0f; //turn off the translation.  Oddly, this is what we want here.
	identityTransform(out);
	LavTransform rot;
	memcpy(rot, t, sizeof(LavTransform));
	rot[0][3]=0;
	rot[1][3]=0;
	rot[2][3]=0;
	transformTranspose(rot, out); //gives us an inverted rotation.
	transformApply(out, trans, trans); //gives us the inverted translation component.
	//now, copy in the inverted translation.
	out[0][3] = trans[0];
	out[1][3] = trans[1];
	out[2][3] = trans[2];
}

Lav_PUBLIC_FUNCTION float vectorDotProduct(LavVector a, LavVector b) {
	return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

Lav_PUBLIC_FUNCTION void vectorCrossProduct(LavVector a, LavVector b, LavVector out) {
	out[0] = a[1]*b[2]-a[2]*b[1];
	out[1]=a[2]*b[0]-a[0]*b[2];
	out[2] = a[0]*b[1]-a[1]*b[0];
}

Lav_PUBLIC_FUNCTION void cameraTransform(LavVector at, LavVector up, LavVector position, LavTransform out) {
	LavVector cy = {up[0], up[1], up[2], 0};
	LavVector cz = {-at[0], -at[1], -at[2]};
	LavVector cx;
	//x is z cross y.
	//but we negated z, so it's actually y cross z.
	vectorCrossProduct(cy, cz, cx);
	//figure out what we need to do with translation.
	float tx, ty, tz;
	tx = -vectorDotProduct(cx, position);
	ty = -vectorDotProduct(cy, position);
	tz = -vectorDotProduct(cz, position);
	//we now put these into out.
	for(int i = 0; i < 3; i++) {
		out[0][i] = cx[i];
		out[1][i] = cy[i];
		out[2][i] = cz[i];
		out[3][i] = 0;
	}
	//and the translation components, computed above:
	out[0][3] = tx;
	out[1][3] = ty;
	out[2][3] = tz;
	out[3][3] = 1;
}

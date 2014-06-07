/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_all.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

/**This is a matrix transformation library adequate for Libaudioverse's purposes**/

Lav_PUBLIC_FUNCTION void identityTransform(LavTransform *out) {
	memset(out->mat, 0, sizeof(LavTransform));
	out->mat[0][0] = 1.0f;
	out->mat[1][1] = 1.0f;
	out->mat[2][2] = 1.0f;
	out->mat[3][3]=1.0f;
}

Lav_PUBLIC_FUNCTION void transformApply(LavTransform trans, LavVector in, LavVector *out) {
	//we're multiplying a 3x3 by a 3x1. Output is therefore 3x1.
	for(int row = 0; row < 4; row++) {
		out->vec[row] = in.vec[0]*trans.mat[row][0]+in.vec[1]*trans.mat[row][1]+in.vec[2]*trans.mat[row][2]+in.vec[3]*trans.mat[row][3];
	}
}

Lav_PUBLIC_FUNCTION void transformMultiply(LavTransform t1, LavTransform t2, LavTransform *out) {
	memset(out->mat, 0, sizeof(float)*16); //make sure it's clear so we can add to it directly.
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			for(int k = 0; k < 4; k++) {	
				out->mat[i][j] += t1.mat[i][k]*t2.mat[k][j];
			}
		}
	}
}

Lav_PUBLIC_FUNCTION void transformTranspose(LavTransform t, LavTransform *out) {
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			out->mat[i][j] = t.mat[j][i];
		}
	}
}

Lav_PUBLIC_FUNCTION void transformInvertOrthoganal(LavTransform t, LavTransform *out) {
	LavTransform tmp = {0};
	LavVector trans = {0};
	trans.vec[0] = -1*t.mat[0][3];
	trans.vec[1] = -1*t.mat[1][3];
	trans.vec[2] = -1*t.mat[2][3];
	trans.vec[3] = 0.0f; //turn off the translation.  Oddly, this is what we want here.
	identityTransform(out);
	LavTransform rot;
	rot = t;
	rot.mat[0][3]=0;
	rot.mat[1][3]=0;
	rot.mat[2][3]=0;
	transformTranspose(rot, out); //gives us an inverted rotation.
	transformApply(*out, trans, &trans); //gives us the inverted translation component.
	//now, copy in the inverted translation.
	out->mat[0][3] = trans.vec[0];
	out->mat[1][3] = trans.vec[1];
	out->mat[2][3] = trans.vec[2];
}

Lav_PUBLIC_FUNCTION float vectorDotProduct(LavVector a, LavVector b) {
	return a.vec[0]*b.vec[0]+a.vec[1]*b.vec[1]+a.vec[2]*b.vec[2];
}

Lav_PUBLIC_FUNCTION void vectorCrossProduct(LavVector a, LavVector b, LavVector *out) {
	out->vec[0] = a.vec[1]*b.vec[2]-a.vec[2]*b.vec[1];
	out->vec[1]=a.vec[2]*b.vec[0]-a.vec[0]*b.vec[2];
	out->vec[2] = a.vec[0]*b.vec[1]-a.vec[1]*b.vec[0];
}

Lav_PUBLIC_FUNCTION void cameraTransform(LavVector at, LavVector up, LavVector position, LavTransform *out) {
	LavVector cy = {.vec = {up.vec[0], up.vec[1], up.vec[2]}};
	LavVector cz = {.vec = {-at.vec[0], -at.vec[1], -at.vec[2]}};
	LavVector cx;
	//x is z cross y.
	//but we negated z, so it's actually y cross z.
	vectorCrossProduct(cy, cz, &cx);
	//figure out what we need to do with translation.
	float tx, ty, tz;
	tx = -vectorDotProduct(cx, position);
	ty = -vectorDotProduct(cy, position);
	tz = -vectorDotProduct(cz, position);
	//we now put these into out.
	for(int i = 0; i < 3; i++) {
		out->mat[0][i] = cx.vec[i];
		out->mat[1][i] = cy.vec[i];
		out->mat[2][i] = cz.vec[i];
		out->mat[3][i] = 0;
	}
	//and the translation components, computed above:
	out->mat[0][3] = tx;
	out->mat[1][3] = ty;
	out->mat[2][3] = tz;
	out->mat[3][3] = 1;
}

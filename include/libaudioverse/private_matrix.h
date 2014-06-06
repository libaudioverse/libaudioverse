/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef float LavTransform[4][4];
typedef float LavVector[4];

Lav_PUBLIC_FUNCTION void identityTransform(LavTransform t);
Lav_PUBLIC_FUNCTION void transformSetTranslation(LavTransform t, LavVector v);
Lav_PUBLIC_FUNCTION void transformGetTranslation(LavTransform t, LavVector out);
Lav_PUBLIC_FUNCTION void transformApply(LavTransform t, LavVector in, LavVector out); //note: if in==out, this violates the aliasing rule and will break optimizers.
Lav_PUBLIC_FUNCTION void transformInvertOrthoganalInPlace(LavTransform t);
Lav_PUBLIC_FUNCTION void transformInvertOrthoganal(LavTransform t, LavTransform out);
Lav_PUBLIC_FUNCTION float vectorDotProduct(LavVector a, LavVector b);
Lav_PUBLIC_FUNCTION void vectorCrossProduct(LavVector a, LavVector b, LavVector out);
Lav_PUBLIC_FUNCTION void cameraTransform(LavTransform t, LavVector at, LavVector up, LavVector position);

#ifdef __cplusplus
}
#endif
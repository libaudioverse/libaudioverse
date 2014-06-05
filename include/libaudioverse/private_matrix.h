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

Lav_PUBLIC_FUNCTION void identityTransform(LavTransform m);
Lav_PUBLIC_FUNCTION void transformSetTranslation(LavTransform m, LavVector v);
Lav_PUBLIC_FUNCTION void transformGetTranslation(LavTransform m, LavVector v);
Lav_PUBLIC_FUNCTION void transformApply(LavTransform m, LavVector in, LavVector out); //note: if in==out, this violates the aliasing rule and will break optimizers.

#ifdef __cplusplus
}
#endif
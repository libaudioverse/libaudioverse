/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "private_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

//Get a sample, with no error checking.
float tableGetSampleFast(LavTable* table, float index);

/**Interpolated tables.

An interpolated table has some properties of ringbuffers, but with floating point math: reads past the end or before the beginning wrap.
It is possible to read between samples; if so, it performs linear interpolation.  All times are in samples, but fractional values are allowed.

Note that these are definitively not LavObjects.  They are intended to be used in tight loops.*/
Lav_PUBLIC_FUNCTION LavError Lav_createTable(LavTable** destination);
Lav_PUBLIC_FUNCTION LavError Lav_tableGetSample(LavTable *table, float index, float* destination);
Lav_PUBLIC_FUNCTION LavError Lav_tableGetSamples(LavTable* table, float index, float delta, unsigned int count, float* destination);
Lav_PUBLIC_FUNCTION LavError Lav_tableSetSamples(LavTable *table, unsigned int count, float* samples);
Lav_PUBLIC_FUNCTION LavError Lav_tableClear(LavTable *table);

#ifdef __cplusplus
}
#endif
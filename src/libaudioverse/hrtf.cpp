/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Read an hrtf file into a LavHrtfData and compute left and right channel HRIR coefficients from an angle.*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_hrtf.hpp>
#include <libaudioverse/private_dspmath.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_errors.hpp>
#include <math.h>

/**Swaps bytes to reverse endianness.*/
void reverse_endianness(char* buffer, unsigned int count, unsigned int window) {
	char* end = buffer+count*window;
	for(; buffer < end; buffer+=window) {
		for(unsigned int i = 0; i < window; i++) {
			char temp = buffer[i];
			buffer[i]=buffer[window-i];
			buffer[window-i]=temp;
		}
	}
}

//this makes sure that we aren't about to do something silently dangerous and tels us at compile time.
static_assert(sizeof(float) == 4, "Sizeof float is not 4; cannot safely work with hrtfs");

LavHrtfData::~LavHrtfData() {
	for(unsigned int i = 0; i < elev_count; i++) {
		delete[] hrirs[i];
	}
	delete[] hrirs;
	delete[] azimuth_counts;
}

unsigned int LavHrtfData::getLength() {
	return hrir_length;
}

void LavHrtfData::loadFromFile(std::string path) {
	//first, load the file if we can.
	FILE *fp = fopen(path.c_str(), "rb");
	if(fp == nullptr) throw LavErrorException(Lav_ERROR_FILE);
	size_t size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	//Okay, load everything.
	char* data = new char[size];

	//do the read.
	size_t read;
	read = fread(data, 1, size, fp);

	//we now handle endianness.
	int32_t endianness_marker = *(int32_t*)data;
	if(endianness_marker != 1) reverse_endianness(data, size, 4);
	//read it again; if it is still not 1, something has gone badly wrong.
	endianness_marker = *(int32_t*)data;
	if(endianness_marker != 1) throw LavErrorException(Lav_ERROR_HRTF_INVALID);

	char* iterator = data;
	const unsigned int window_size = 4;

	//read the header information.
	iterator += window_size;//skip the endianness marker, which is handled above.
	samplerate = *(int32_t*)iterator;

	iterator += window_size;
	hrir_count = *(int32_t*)iterator;
	iterator += window_size;
	elev_count = *(int32_t*)iterator;
	iterator += window_size;
	min_elevation = *(int32_t*)iterator;
	iterator += window_size;
	max_elevation = *(int32_t*)iterator;
	iterator += window_size;

	//this is the first "dynamic" piece of information.
	azimuth_counts = new unsigned int[elev_count];
	for(unsigned int i = 0; i < elev_count; i++) {
		azimuth_counts[i] = *(int32_t*)iterator;
		iterator += window_size;
	}

	//sanity check: we must have as many hrirs as the sum of the above array.
	int32_t sum_sanity_check = 0;
	for(unsigned int i = 0; i < elev_count; i++) sum_sanity_check +=azimuth_counts[i];
	if(sum_sanity_check != hrir_count) throw LavErrorException(Lav_ERROR_HRTF_INVALID);

	hrir_length = *(int32_t*)iterator;
	iterator += window_size;

	unsigned int size_so_far = iterator-data;
	size_t size_remaining = size-size_so_far;
	//we must have enough remaining to be all hrir hrirs.
	size_t hrir_size = hrir_length*hrir_count*sizeof(float);
	if(hrir_size != size_remaining) throw LavErrorException(Lav_ERROR_HRTF_INVALID);

	//last step.  Initialize the HRIR array.
	hrirs = new float**[elev_count];
	//do the azimuth dimension.
	for(unsigned int i = 0; i < elev_count; i++) {
		hrirs[i] = new float*[azimuth_counts[i]];
	}

	//the above gives us what amounts to a 2d array.  The first dimension represents elevation.  The second dimension represents azimuth going clockwise.
	//fill it.
	for(unsigned int elev = 0; elev < elev_count; elev++) {
		for(unsigned int azimuth = 0; azimuth < azimuth_counts[elev]; azimuth++) {
			hrirs[elev][azimuth] = new float[hrir_length];
			memcpy(hrirs[elev][azimuth], iterator, hrir_length*sizeof(float));
			iterator+=hrir_length*sizeof(float);
		}
	}
}

//a complete HRTF for stereo is two calls to this function.
void LavHrtfData::computeCoefficientsMono(float elevation, float azimuth, float* out) {
	//clamp the elevation.
	if(elevation < min_elevation) {elevation = (float)min_elevation;}
	else if(elevation > max_elevation) {elevation = (float)max_elevation;}

	//we need to convert the elevation into an index.  First, truncate it to an integer (which rounds towards 0).
	int truncatedElevation = (int)truncf(elevation);
	//we now need to know how many degrees there are per elevation increase/decrease.  This is a bit tricky and, if done wrong, will result in an off-by-one.
	int degreesPerElevation = 0;
	if(min_elevation != max_elevation) { //it's not 0, it has to be something else, and the count has to be at least 2.
		//it's the difference between min and max, dividedd by the count.  The count includes 0, however, so we have to subtract one from it.
		degreesPerElevation = (max_elevation-min_elevation)/(elev_count-1);
	}

	//we have a truncated elevation.  We now simply take an integer division, or assume it's 0.
	int elevationIndex = degreesPerElevation ? truncatedElevation/degreesPerElevation : 0;
	//this is relative to whatever index happens to be "0", that is it is an offset from the 0 index.  We have to offset it upwards so it's not negative.
	int elevationIndexOffset = degreesPerElevation ? abs(min_elevation)/degreesPerElevation : 0;
	elevationIndex += elevationIndexOffset;

	//ElevationIndex lets us get an array of azimuth coefficients.  Go ahead and pull it out now, so we can conceptually forget about all the above variables.
	float** azimuths = hrirs[elevationIndex];
	unsigned int azimuthCount = azimuth_counts[elevationIndex];
	float degreesPerAzimuth = 360.0f/azimuthCount;

	unsigned int azimuthIndex1, azimuthIndex2;
	azimuthIndex1 = (unsigned int)floorf(azimuth/degreesPerAzimuth);
	azimuthIndex2 = azimuthIndex1+1;
	float azimuthWeight1, azimuthWeight2;
	//this is the same logic as a bunch of other places, with a minor variation.
	azimuthWeight1 = ceilf(azimuthIndex2*degreesPerAzimuth-azimuth)/degreesPerAzimuth;
	azimuthWeight2 = floorf(azimuth-azimuthIndex1*degreesPerAzimuth)/degreesPerAzimuth;
	//now that we have some weights, we need to ringmod the azimuth indices. 360==0 in trig, but causes one of them to go past the end.
	azimuthIndex1 = ringmodi(azimuthIndex1, azimuthCount);
	azimuthIndex2 = ringmodi(azimuthIndex2, azimuthCount);

	//this is probably the only part of this that can't go wrong, assuming the above calculations are all correct.  Interpolate between the two azimuths.
	for(unsigned int i = 0; i < hrir_length; i++) {
		out[i] = azimuthWeight1*azimuths[azimuthIndex1][i]+azimuthWeight2*azimuths[azimuthIndex2][i];
	}
}

void LavHrtfData::computeCoefficientsStereo(float elevation, float azimuth, float *left, float* right) {
	//wrap azimuth to be > 0 and < 360.
	azimuth = ringmodf(azimuth, 360.0f);
	//the hrtf datasets are right ear coefficients.  Consequently, the right ear requires no changes.
	computeCoefficientsMono(elevation, azimuth, right);
	//the left ear is found at an azimuth which is reflectred about 0 degrees.
	azimuth = ringmodf(360-azimuth, 360.0f);
	computeCoefficientsMono(elevation, azimuth, left);
}

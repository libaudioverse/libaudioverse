/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Read an hrtf file into a LavHrtfData and compute left and right channel HRIR coefficients from an angle.*/
#define _CRT_SECURE_NO_WARNINGS //disables warnings about fopen that occur only with microsoft compilers.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <libaudioverse/private_all.h>
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
_Static_assert(sizeof(float) == 4, "Sizeof float is not 4; cannot safely work with hrtfs");

Lav_PUBLIC_FUNCTION LavError Lav_createHrtfData(const char* path, LavHrtfData** destination) {
	//first, load the file if we can.
	FILE *fp = fopen(path, "rb");
	if(fp == NULL) {
		return Lav_ERROR_FILE_NOT_FOUND;
	}

	size_t size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if(size == 0) return Lav_ERROR_HRTF_INVALID;

	//Okay, load everything.
	char* data = malloc(size);
	if(data == NULL) {
		return Lav_ERROR_MEMORY;
	}

	//do the read.
	size_t read;
	read = fread(data, 1, size, fp);
	if(read != size) return Lav_ERROR_FILE;
	fclose(fp);

	//we now handle endianness.
	int32_t endianness_marker = *(int32_t*)data;
	if(endianness_marker != 1) reverse_endianness(data, size, 4);
	//read it again; if it is still not 1, something has gone badly wrong.
	endianness_marker = *(int32_t*)data;
	if(endianness_marker != 1) return Lav_ERROR_HRTF_CORRUPT;

	char* iterator = data;
	const unsigned int window_size = 4;

	//read the header information.
	iterator += window_size;//skip the endianness marker, which is handled above.
	int32_t samplerate = *(int32_t*)iterator;

	iterator += window_size;
	int32_t number_of_hrirs = *(int32_t*)iterator;
	iterator += window_size;
	int32_t number_of_elevations = *(int32_t*)iterator;
	iterator += window_size;
	int32_t minimum_elevation = *(int32_t*)iterator;
	iterator += window_size;
	int32_t maximum_elevation = *(int32_t*)iterator;
	iterator += window_size;

	//do some sanity checks.
	if(number_of_hrirs <= 0) {return Lav_ERROR_HRTF_INVALID;}
	if(number_of_elevations <= 0) {return Lav_ERROR_HRTF_INVALID;}
	if(number_of_elevations != 1 && minimum_elevation == maximum_elevation) {return Lav_ERROR_HRTF_INVALID;}

	//this is the first "dynamic" piece of information.
	int32_t  *azimuths_per_elevation = malloc(sizeof(int32_t)*number_of_elevations);
	for(int i = 0; i < number_of_elevations; i++) {
		azimuths_per_elevation[i] = *(int32_t*)iterator;
		iterator += window_size;
	}

	//sanity check: we must have as many hrirs as the sum of the above array.
	int32_t sum_sanity_check = 0;
	for(int i = 0; i < number_of_elevations; i++) sum_sanity_check +=azimuths_per_elevation[i];
	if(sum_sanity_check != number_of_hrirs) {return Lav_ERROR_HRTF_INVALID;}

	int32_t hrir_length = *(int32_t*)iterator;
	iterator += window_size;

	unsigned int size_so_far = iterator-data;
	size_t size_remaining = size-size_so_far;
	//we must have enough remaining to be all hrir hrirs.
	size_t hrir_size = hrir_length*number_of_hrirs*sizeof(float);
	if(hrir_size != size_remaining) {return Lav_ERROR_HRTF_CORRUPT;} //justification: it's probably missing data, not invalid data.

	LavHrtfData *hrtf = calloc(1, sizeof(LavHrtfData));
	hrtf->elev_count = number_of_elevations;
	hrtf->hrir_count = number_of_hrirs;
	hrtf->hrir_length = hrir_length;
	hrtf->samplerate = samplerate;
	hrtf->min_elevation = minimum_elevation;
	hrtf->max_elevation = maximum_elevation;

	hrtf->azimuth_counts = calloc(number_of_elevations, sizeof(unsigned int));
	if(hrtf->azimuth_counts == NULL) return Lav_ERROR_MEMORY;
	for(int i = 0; i < number_of_elevations; i++) hrtf->azimuth_counts[i] = azimuths_per_elevation[i];

	//last step.  Initialize the HRIR array.
	hrtf->hrirs = malloc(sizeof(float**)*number_of_elevations); //elevation dimension.
	if(hrtf->hrirs == NULL) {return Lav_ERROR_MEMORY;}
	//do the azimuth dimension.
	for(int i = 0; i < number_of_elevations; i++) {
		hrtf->hrirs[i] = malloc(azimuths_per_elevation[i]*sizeof(float*));
		if(hrtf->hrirs[i] == NULL) {return Lav_ERROR_MEMORY;}
	}

	//the above gives us what amounts to a 2d array.  The first dimension represents elevation.  The second dimension represents azimuth going clockwise.
	//fill it.
	for(int elev = 0; elev < number_of_elevations; elev++) {
		for(int azimuth = 0; azimuth < azimuths_per_elevation[elev]; azimuth++) {
			hrtf->hrirs[elev][azimuth] = malloc(sizeof(float)*hrir_length);
			if(hrtf->hrirs[elev][azimuth] == NULL) {return Lav_ERROR_NONE;}
			memcpy(hrtf->hrirs[elev][azimuth], iterator, hrir_length*sizeof(float));
			iterator+=hrir_length*sizeof(float);
		}
	}

	*destination = hrtf;
	return Lav_ERROR_NONE;
}

//a complete HRTF for stereo is two calls to this function.
Lav_PUBLIC_FUNCTION void hrtfComputeCoefficientsMono(LavHrtfData *hrtf, float elevation, float azimuth, float* out) {
	//clamp the elevation.
	if(elevation < hrtf->min_elevation) {elevation = (float)hrtf->min_elevation;}
	else if(elevation > hrtf->max_elevation) {elevation = (float)hrtf->max_elevation;}

	//we need to convert the elevation into an index.  First, truncate it to an integer (which rounds towards 0).
	int truncatedElevation = (int)truncf(elevation);
	//we now need to know how many degrees there are per elevation increase/decrease.  This is a bit tricky and, if done wrong, will result in an off-by-one.
	int degreesPerElevation = 0;
	if(hrtf->min_elevation != hrtf->max_elevation) { //it's not 0, it has to be something else, and the count has to be at least 2.
		//it's the difference between min and max, dividedd by the count.  The count includes 0, however, so we have to subtract one from it.
		degreesPerElevation = (hrtf->max_elevation-hrtf->min_elevation)/(hrtf->elev_count-1);
	}

	//we have a truncated elevation.  We now simply take an integer division, or assume it's 0.
	int elevationIndex = degreesPerElevation ? truncatedElevation/degreesPerElevation : 0;
	//this is relative to whatever index happens to be "0", that is it is an offset from the 0 index.  We have to offset it upwards so it's not negative.
	int elevationIndexOffset = degreesPerElevation ? abs(hrtf->min_elevation)/degreesPerElevation : 0;
	elevationIndex += elevationIndexOffset;

	//ElevationIndex lets us get an array of azimuth coefficients.  Go ahead and pull it out now, so we can conceptually forget about all the above variables.
	float** azimuths = hrtf->hrirs[elevationIndex];
	unsigned int azimuthCount = hrtf->azimuth_counts[elevationIndex];
	float degreesPerAzimuth = 360.0f/azimuthCount;

	unsigned int azimuthIndex1, azimuthIndex2;
	azimuthIndex1 = (unsigned int)floorf(azimuth/degreesPerAzimuth);
	azimuthIndex2 = (unsigned int)ceilf(azimuth/degreesPerAzimuth);
	float azimuthWeight1, azimuthWeight2;
	//this is the same logic as a bunch of other places, with a minor variation.
	azimuthWeight1 = ceilf(azimuthIndex2*degreesPerAzimuth-azimuth)/degreesPerAzimuth;
	azimuthWeight2 = floorf(azimuth-azimuthIndex1*degreesPerAzimuth)/degreesPerAzimuth;

	//this is probably the only part of this that can't go wrong, assuming the above calculations are all correct.  Interpolate between the two azimuths.
	for(unsigned int i = 0; i < hrtf->hrir_length; i++) {
		out[i] = azimuthWeight1*azimuths[azimuthIndex1][i]+azimuthWeight2*azimuths[azimuthIndex2][i];
	}
}

Lav_PUBLIC_FUNCTION void computeHrtfCoefficients(LavHrtfData *hrtf, float elevation, float azimuth, float *left, float* right) {
	//wrap azimuth to be > 0 and < 360.
	while(azimuth < 0) azimuth += 360.0f;
	while(azimuth > 360.0f) azimuth -= 360.0f;
	//the hrtf datasets are right ear coefficients.  Consequently, the right ear requires no changes.
	hrtfComputeCoefficientsMono(hrtf, elevation, azimuth, right);
	//the left ear is found at an azimuth which is reflectred about 0 degrees.
	//by the while loops above, azimuth can't be >360 or <0, so it necessarily follows that this is >0 and <360.
	azimuth = 360-azimuth;
	hrtfComputeCoefficientsMono(hrtf, elevation, azimuth, left);
}

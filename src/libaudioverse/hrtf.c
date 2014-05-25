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

//this makes sure that we aren't about to do something silently dangerous.
_Static_assert(sizeof(float) == 4, "Sizeof float is not 4; cannot safely work with hrtfs");

Lav_PUBLIC_FUNCTION LavError Lav_createHrtfTable(const char* path, LavHrtfData** destination) {
	//first, load the file if we can.
	FILE *fp = fopen(path, "rb");
	if(fp == NULL) {
		return Lav_ERROR_FILE_NOT_FOUND;
	}

	size_t size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	if(size == 0) return Lav_ERROR_HRTF_INVALID;

	//Okay, load everything.
	char* data = malloc(size);
	if(data == NULL) {
		return Lav_ERROR_MEMORY;
	}

	//do the read.
	size_t read;
	read = 0;
	while(read < size) read += fread(data, 1, size, fp);
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
	int32_t number_of_responses = *(int32_t*)iterator;
	iterator += window_size;
	int32_t number_of_elevations = *(int32_t*)iterator;
	iterator += window_size;
	int32_t minimum_elevation = *(int32_t*)iterator;
	iterator += window_size;
	int32_t maximum_elevation = *(int32_t*)iterator;
	iterator += window_size;

	//do some sanity checks.
	if(number_of_responses <= 0) {return Lav_ERROR_HRTF_INVALID;}
	if(number_of_elevations <= 0) {return Lav_ERROR_HRTF_INVALID;}
	if(number_of_elevations != 1 && minimum_elevation == maximum_elevation) {return Lav_ERROR_HRTF_INVALID;}

	//this is the first "dynamic" piece of information.
	int32_t  *azimuths_per_elevation = malloc(sizeof(int32_t)*number_of_elevations);
	for(int i = 0; i < number_of_elevations; i++) {
		azimuths_per_elevation[i] = *(int32_t*)iterator;
		iterator += window_size;
	}

	//sanity check: we must have as many responses as the sum of the above array.
	int32_t sum_sanity_check = 0;
	for(int i = 0; i < number_of_elevations; i++) sum_sanity_check +=azimuths_per_elevation[i];
	if(sum_sanity_check != number_of_responses) {return Lav_ERROR_HRTF_INVALID;}

	int32_t response_length = *(int32_t*)iterator;
	iterator += window_size;

	unsigned int size_so_far = iterator-data;
	size_t size_remaining = size-size_so_far;
	//we must have enough remaining to be all hrir responses.
	size_t response_size = response_length*number_of_responses;
	if(response_size != size_remaining) {return Lav_ERROR_HRTF_CORRUPT;} //justification: it's probably missing data, not invalid data.

	//Allocate and fill out a LavHrtfData.
	LavHrtfData *hrtf = calloc(1, sizeof(LavHrtfData));
	return Lav_ERROR_NONE;
}

/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Read an hrtf file into a HrtfData and compute left and right channel HRIR coefficients from an angle.*/
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/hrtf.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/error.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/data.hpp>
#include <libaudioverse/private/memory.hpp>
#include <math.h>
#include <memory>
#include <algorithm>
#include <kiss_fftr.h>

namespace libaudioverse_implementation {

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

HrtfData::~HrtfData() {
	if(temporary_buffer1) freeArray(temporary_buffer1);
	if(temporary_buffer2) freeArray(temporary_buffer2);
	if(hrirs == nullptr) return; //we never loaded one.
	for(int i = 0; i < elev_count; i++) {
		//The staticResamplerKernel allocates with new[], not allocArray.
		for(int j = 0; j < azimuth_counts[i]; j++) delete[] hrirs[i][j];
		delete[] hrirs[i];
	}
	delete[] hrirs;
	delete[] azimuth_counts;
	freeArray(fft_time_data);
	freeArray(fft_data);
	kiss_fftr_free(fft);
	kiss_fftr_free(ifft);
}

void HrtfData::linearPhase(float* buffer) {
	//Note that this is in effect circular convolution with a truncation.
	std::copy(buffer, buffer+hrir_length, fft_time_data);
	std::fill(fft_time_data+hrir_length, fft_time_data+hrir_length*2, 0.0f);
	kiss_fftr(fft, fft_time_data, fft_data);
	for(int i = 0; i < hrir_length+1; i++) {
		fft_data[i].r = cabs(fft_data[i].r, fft_data[i].i);
		fft_data[i].i = 0.0f;
	}
	kiss_fftri(ifft, fft_data, fft_time_data);
	//Apply the downscaling that kissfft requires, otherwise this is too loud.
	//Also accomplish copying back to the starting piont.
	scalarMultiplicationKernel(hrir_length, 1.0f/(2*hrir_length), fft_time_data, buffer);
}

int HrtfData::getLength() {
	return hrir_length;
}

void HrtfData::loadFromFile(std::string path, unsigned int forSr) {
	//first, load the file if we can.
	FILE *fp = fopen(path.c_str(), "rb");
	if(fp == nullptr) ERROR(Lav_ERROR_FILE, std::string("Could not find HRTF file ")+path);
	size_t size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	//Okay, load everything.
	char* data = new char[size];

	//do the read.
	size_t read;
	read = fread(data, 1, size, fp);
	loadFromBuffer(read, data, forSr);
	delete[] data;
}

void HrtfData::loadFromDefault(unsigned int forSr) {
	loadFromBuffer(default_hrtf_size, default_hrtf, forSr);
}

#define convi(b) safeConvertMemory<int32_t>(b)
#define convf(b) safeConvertMemory<float>*(b)

void HrtfData::loadFromBuffer(unsigned int length, char* buffer, unsigned int forSr) {
	//we now handle endianness.
	int32_t endianness_marker =convi(buffer);
	if(endianness_marker != 1) reverse_endianness(buffer, length, 4);
	//read it again; if it is still not 1, something has gone badly wrong.
	endianness_marker = convi(buffer);
	if(endianness_marker != 1) ERROR(Lav_ERROR_HRTF_INVALID, "Could not correct endianness for this architecture.");

	char* iterator = buffer;
	const unsigned int window_size = 4;

	//read the header information.
	iterator += window_size;//skip the endianness marker, which is handled above.
	samplerate = convi(iterator);

	iterator += window_size;
	hrir_count = convi(iterator);
	iterator += window_size;
	elev_count = convi(iterator);
	iterator += window_size;
	min_elevation = convi(iterator);
	iterator += window_size;
	max_elevation = convi(iterator);
	iterator += window_size;

	//this is the first "dynamic" piece of information.
	azimuth_counts = new int[elev_count];
	for(int i = 0; i < elev_count; i++) {
		azimuth_counts[i] = convi(iterator);
		iterator += window_size;
	}

	//sanity check: we must have as many hrirs as the sum of the above array.
	int32_t sum_sanity_check = 0;
	for(int i = 0; i < elev_count; i++) sum_sanity_check +=azimuth_counts[i];
	if(sum_sanity_check != hrir_count) ERROR(Lav_ERROR_HRTF_INVALID, "Not enough or too many responses.");

	int before_hrir_length = convi(iterator);
	iterator += window_size;

	unsigned int length_so_far = iterator-buffer;
	size_t size_remaining = length-length_so_far;
	//we must have enough remaining to be all hrir hrirs.
	size_t hrir_size = before_hrir_length*hrir_count*sizeof(float);
	if(hrir_size != size_remaining) ERROR(Lav_ERROR_HRTF_INVALID, "Not enough HRIR data.");

	//last step.  Initialize the HRIR array.
	hrirs = new float**[elev_count];
	//do the azimuth dimension.
	for(int i = 0; i < elev_count; i++) {
		hrirs[i] = new float*[azimuth_counts[i]];
	}

	//the above gives us what amounts to a 2d array.  The first dimension represents elevation.  The second dimension represents azimuth going clockwise.
	//fill it.
	float* tempBuffer = allocArray<float>(before_hrir_length);
	int final_hrir_length = 0;
	for(int elev = 0; elev < elev_count; elev++) {
		for(int azimuth = 0; azimuth < azimuth_counts[elev]; azimuth++) {
			memcpy(tempBuffer, iterator, sizeof(float)*before_hrir_length);
			staticResamplerKernel(samplerate, forSr, 1, before_hrir_length, tempBuffer, &final_hrir_length, &hrirs[elev][azimuth]);
			iterator+=before_hrir_length*sizeof(float);
		}
	}
	hrir_length = final_hrir_length;
	samplerate = forSr;
	freeArray(tempBuffer);

	if(temporary_buffer1) freeArray(temporary_buffer1);
	if(temporary_buffer2) freeArray(temporary_buffer2);
	temporary_buffer1 = allocArray<float>(hrir_length);
	temporary_buffer2 = allocArray<float>(hrir_length);
	
	//stuff for linear phase filters.
	fft_time_data = allocArray<float>(hrir_length*2);
	fft_data = allocArray<kiss_fft_cpx>(hrir_length+1); //half the bins, plus dc.
	fft = kiss_fftr_alloc(hrir_length*2, 0, nullptr, nullptr);
	ifft = kiss_fftr_alloc(hrir_length*2, 1, nullptr, nullptr);
}

//a complete HRTF for stereo is two calls to this function.
//some final preparation is done afterwords.
//This is very complicated, thus the heavy commenting.
//todo: can this be made simpler?
void HrtfData::computeCoefficientsMono(float elevation, float azimuth, float* out, bool linphase) {
	//clamp the elevation.
	if(elevation < min_elevation) {elevation = (float)min_elevation;}
	else if(elevation > max_elevation) {elevation = (float)max_elevation;}

	//we need to convert the elevation into an index.  First, truncate it to an integer (which rounds towards 0).
	int truncatedElevation = (int)truncf(elevation);
	//we now need to know how many degrees there are per elevation increase/decrease.  This is a bit tricky and, if done wrong, will result in an off-by-one.
	int degreesPerElevation = 0;
	if(min_elevation != max_elevation) { //it's not 0, it has to be something else, and the count has to be at least 2.
		//it's the difference between min and max, dividedd by the count.
		//The count includes both endpoints of an interval, as well as  all the marks between.
		//We subtract 1 because we're not counting points, we're counting spaces between points.
		degreesPerElevation = (max_elevation-min_elevation)/(elev_count-1);
	}

	//we have a truncated elevation.  We now simply take an integer division, or assume it's 0.
	//This is an array because We need to do the vertical crossfade in this function.
	int elevationIndex[2];
	elevationIndex[0] = degreesPerElevation ? truncatedElevation/degreesPerElevation : 0;
	//this is relative to whatever index happens to be "0", that is it is an offset from the 0 index.  We have to offset it upwards so it's not negative.
	int elevationIndexOffset = degreesPerElevation ? abs(min_elevation)/degreesPerElevation : 0;
	elevationIndex[0] += elevationIndexOffset;
	elevationIndex[1] = std::min(elevationIndex[0]+1, elev_count-1);
	double elevationWeights[2];
	float ringmoddedElevation = ringmodf(elevation, degreesPerElevation);
	if(ringmoddedElevation < 0) ringmoddedElevation =degreesPerElevation-ringmoddedElevation;
	if(ringmoddedElevation > degreesPerElevation) ringmoddedElevation = degreesPerElevation;
	elevationWeights[0] = (degreesPerElevation-ringmoddedElevation)/degreesPerElevation;
	elevationWeights[1] = ringmoddedElevation/degreesPerElevation;

	memset(out, 0, sizeof(float)*hrir_length);
	for(int i = 0; i < 2; i++) {
		//ElevationIndex lets us get an array of azimuth coefficients.  Go ahead and pull it out now, so we can conceptually forget about all the above variables.
		float** azimuths = hrirs[elevationIndex[i]];
		int azimuthCount = azimuth_counts[elevationIndex[i]];
		float degreesPerAzimuth = 360.0f/azimuthCount;
		int azimuthIndex1, azimuthIndex2;
		azimuthIndex1 = (int)floorf(azimuth/degreesPerAzimuth);
		azimuthIndex2 = azimuthIndex1+1;
		float azimuthWeight1, azimuthWeight2;
		//this is the same logic as a bunch of other places, with a minor variation.
		azimuthWeight1 = ceilf(azimuthIndex2*degreesPerAzimuth-azimuth)/degreesPerAzimuth;
		azimuthWeight2 = floorf(azimuth-azimuthIndex1*degreesPerAzimuth)/degreesPerAzimuth;
		//now that we have some weights, we need to ringmod the azimuth indices. 360==0 in trig, but causes one of them to go past the end.
		azimuthIndex1 = ringmodi(azimuthIndex1, azimuthCount);
		azimuthIndex2 = ringmodi(azimuthIndex2, azimuthCount);

		//this is probably the only part of this that can't go wrong, assuming the above calculations are all correct.  Interpolate between the two azimuths.
		if(linphase == false) {
			for(int j = 0; j < hrir_length; j++) {
				out[j] += elevationWeights[i]*(azimuthWeight1*azimuths[azimuthIndex1][j]+azimuthWeight2*azimuths[azimuthIndex2][j]);
			}
		}
		//otherwise, the slower version; this involves copies and the fft.
		else {
			std::copy(azimuths[azimuthIndex1], azimuths[azimuthIndex1]+hrir_length, temporary_buffer1);
			std::copy(azimuths[azimuthIndex2], azimuths[azimuthIndex2]+hrir_length, temporary_buffer2);
			linearPhase(temporary_buffer1);
			linearPhase(temporary_buffer2);
			for(int j = 0; j < hrir_length; j++) {
				out[j] += elevationWeights[i]*(temporary_buffer1[j]*azimuthWeight1+temporary_buffer2[j]*azimuthWeight2);
			}
		}
	}
}

void HrtfData::computeCoefficientsStereo(float elevation, float azimuth, float *left, float* right, bool linphase) {
	//wrap azimuth to be > 0 and < 360.
	azimuth = ringmodf(azimuth, 360.0f);
	//the hrtf datasets are right ear coefficients.  Consequently, the right ear requires no changes.
	computeCoefficientsMono(elevation, azimuth, right, linphase);
	//the left ear is found at an azimuth which is reflectred about 0 degrees.
	azimuth = ringmodf(360-azimuth, 360.0f);
	computeCoefficientsMono(elevation, azimuth, left, linphase);
}

}
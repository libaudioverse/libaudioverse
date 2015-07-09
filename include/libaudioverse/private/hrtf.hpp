/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <string>
#include <memory>
#include <kiss_fftr.h>

namespace libaudioverse_implementation {

class HrtfData {
	public:
	~HrtfData();
	//get the appropriate coefficients for one channel.  A stereo hrtf is two calls to this function.
	//If linphase is true, convert to a  linear phase filter (keeping only amplitude response).
	void computeCoefficientsMono(float elevation, float azimuth, float* out, bool linphase=false);

	//warning: writes directly to the output destination, doesn't allocate a new one.
	//linphase is the same as computeCoefficientsMono.
	void computeCoefficientsStereo(float elevation, float azimuth, float* left, float* right, bool linphase = false);

	//load from a file.
	void loadFromFile(std::string path, unsigned int forSr);
	void loadFromDefault(unsigned int forSr);
	void loadFromBuffer(unsigned int length, char* buffer, unsigned int forSr);

	//Linear phase an hrir response, usually stored in a temporary bufer.
	void linearPhase(float* buffer);
	
	//get the hrir's length.
	int getLength();
	private:
	int elev_count = 0, hrir_count = 0, hrir_length = 0;
	int min_elevation = 0, max_elevation = 0;
	int *azimuth_counts = nullptr;
	int samplerate = 0;
	float ***hrirs = nullptr;
	//used for crossfading so we don't clobber the heap.
	float *temporary_buffer1 = nullptr, *temporary_buffer2 = nullptr;
	//used when we need a minimum phase conversion.
	kiss_fftr_cfg  fft = nullptr, ifft = nullptr;
	kiss_fft_cpx* fft_data = nullptr;
	float* fft_time_data = nullptr; //needed as a temporary buffer, because technically this is circular convolution.
};

}
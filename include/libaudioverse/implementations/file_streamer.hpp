/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/file.hpp"
#include <speex_resampler_cpp.hpp>
#include <memory>
#include <algorithm>
#include <string>
#include <inttypes.h>

namespace libaudioverse_implementation {

/**This is a lot like a BufferPlayer, but channels matches the file always and we resample at run-time.*/

class FileStreamer {
	public:
	FileStreamer(std::string path, int _block_size, float _sr);
	~FileStreamer();
	void process(float** outputs);
	void setPosition(double position);
	double getPosition();
	double getDuration();
	void setIsLooping(bool l);
	bool getIsLooping();
	//Increments every time the buffer ends.
	//Only true if and only if we aren't looping, otherwise false.
	//This last fact is important for the node's process method.
	//Also, this can be significantly (on the order of up to 100 MS) ahead of schedule.
	bool getEnded();
	private:
	void feedResampler();
	int block_size = 0;
	float sr = 0.0f;
	FileReader reader;
	std::shared_ptr<speex_resampler_cpp::Resampler> resampler = nullptr;
	float *workspace_before_resampling = nullptr, *workspace_after_resampling = nullptr;
	int channels = 0;
	double position = 0.0, duration  = 0.0, position_per_sample = 0.0;
	int64_t position_in_frames = 0;
	bool is_looping = false, ended = false;
};

}
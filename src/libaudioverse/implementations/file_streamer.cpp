/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/file_streamer.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <speex_resampler_cpp.hpp>
#include <algorithm>
#include <inttypes.h>

namespace libaudioverse_implementation {

FileStreamer::FileStreamer(std::string path, int _block_size, float _sr):
block_size(_block_size), sr(_sr) {
	reader.open(path.c_str());
	duration = reader.getFrameCount()/(double)reader.getSr();
	position_per_sample = 1.0/reader.getSr();
	resampler = speex_resampler_cpp::createResampler(block_size, reader.getChannelCount(), reader.getSr(), sr);
	workspace_before_resampling = allocArray<float>(block_size*reader.getChannelCount());
	workspace_after_resampling = allocArray<float>(block_size*reader.getChannelCount());
}

FileStreamer::~FileStreamer() {
	freeArray(workspace_before_resampling);
	freeArray(workspace_after_resampling);
}

void FileStreamer::process(float** outputs) {
	int got = 0;
	float* ptr = workspace_after_resampling;
	while(got < block_size) {
		int gotThisIteration = resampler->write(workspace_after_resampling, block_size-got);
		if(gotThisIteration == 0 && ended == false) feedResampler();
		else if(gotThisIteration == 0) break;
		got += gotThisIteration;
		ptr += gotThisIteration*reader.getChannelCount();
	}
	std::fill(ptr, workspace_after_resampling+block_size*reader.getChannelCount(), 0.0f);
	uninterleaveSamples(reader.getChannelCount(), block_size, workspace_after_resampling, reader.getChannelCount(), outputs);
}

void FileStreamer::feedResampler() {
	//First, write stuff to the workspace, of size _block_size.
	unsigned int needed = block_size;
	unsigned int got = 0;
	float* ptr = workspace_before_resampling;
	while(needed) {
		got = reader.read(needed, ptr);
		ptr += got*reader.getChannelCount();
		needed -= got;
		position += got*position_per_sample;
		//We didn't get frames, the duration isn't 0 frames, and we're looping.
		//Libsndfile doesn't let us ask why, and says we're supposed to just assume that this means the end.
		if(got == 0 && reader.getFrameCount() && is_looping) {
			position = 0.0;
			reader.seek(0);
		}
		else if(got == 0) { //This is probably the end, and Libsndfile annoyingly doesn't make it clear.
			ended = true;
			break;
		}
	}
	std::fill(ptr, workspace_before_resampling+block_size*reader.getChannelCount(), 0.0f);
	//And then feed the resampler.
	resampler->read(workspace_before_resampling);
}

void FileStreamer::setPosition(double position) {
	position = std::min(position, duration);
	reader.seek(reader.getSr()*position);
	this->position = position;
	ended = false;
}

double FileStreamer::getPosition() {
	return position;
}

double FileStreamer::getDuration() {
	return duration;
}

void FileStreamer::setIsLooping(bool l) {
	is_looping = l;
	if(l) ended = false;
}

bool FileStreamer::getIsLooping() {
	return is_looping;
}

bool FileStreamer::getEnded() {
	return ended;
}

}
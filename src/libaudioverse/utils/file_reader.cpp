/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_file.hpp>
#include <libaudioverse/libaudioverse.h>
#include <sndfile.h>
#include <libaudioverse/private_errors.hpp>

LavFileReader::~LavFileReader() {
	if(handle) close(); //make sure the file gets closed behind us.
}

void LavFileReader::open(const char* path) {
	if(handle) throw LavErrorException(Lav_ERROR_FILE);
	handle = sf_open(path, SFM_READ, &info);
	if(handle == nullptr) {
		throw LavErrorException(Lav_ERROR_FILE);
	}
}

void LavFileReader::close() {
	if(handle != NULL)  {
		sf_close(handle);
		handle = NULL;
		info = {0};
	} else {
		throw LavErrorException(Lav_ERROR_FILE);
	}
}

float LavFileReader::getSr() {
	return (float)info.samplerate;
}

unsigned int LavFileReader::getChannelCount() {
	return info.channels;
}

unsigned int LavFileReader::getFrameCount() {
	return (unsigned int)info.frames;
}

unsigned int LavFileReader::getSampleCount() {
	return getChannelCount()*getFrameCount();
}

unsigned int LavFileReader::read(unsigned int frames, float* buffer) {
	if(handle == NULL) return 0;
	return (unsigned int)sf_readf_float(handle, buffer, frames);
}

unsigned int LavFileReader::readAll(float* buffer) {
	return read(getFrameCount(), buffer);
}

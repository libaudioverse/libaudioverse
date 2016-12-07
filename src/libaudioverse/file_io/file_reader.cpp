/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
//This has to be first for the macro.
#ifdef WIN32
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/error.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/utf8.hpp>
#include <inttypes.h>
#include <cstddef>


namespace libaudioverse_implementation {

FileReader::~FileReader() {
	if(handle) close(); //make sure the file gets closed behind us.
}

void FileReader::open(const char* path) {
	if(handle) ERROR(Lav_ERROR_FILE, "Attempt to open file without closing.");
	#ifdef WIN32
	std::string p(path);
	auto wp = utf8ToWide(p);
	handle = sf_wchar_open(wp.c_str(), SFM_READ, &info);
	#else
	handle = sf_open(path, SFM_READ, &info);
	#endif
	if(handle == nullptr) {
		ERROR(Lav_ERROR_FILE, std::string(path)+" not found or invalid format.");
	}
}

void FileReader::openFromBuffer(char* buffer, int64_t len) {
	if(buffer == nullptr) ERROR(Lav_ERROR_RANGE, "Buffer must not be null.");
	if(len <= 0) ERROR(Lav_ERROR_RANGE, "Length of buffer must be positive.");
	buffer_wrapper = new LibsndfileBufferWrapper(buffer, len);
	buffer_wrapper->open(SFM_READ, &handle, &info);
	if(handle == nullptr) {
		delete buffer_wrapper;
		ERROR(Lav_ERROR_FILE, "Buffer could not be decoded.");
	}
}


void FileReader::close() {
	if(handle != NULL)  {
		sf_close(handle);
		handle = NULL;
		info = {0};
		if(buffer_wrapper) {delete buffer_wrapper;}
	} else {
		ERROR(Lav_ERROR_FILE, "Attempt to close file without opening first.");
	}
}

float FileReader::getSr() {
	return (float)info.samplerate;
}

unsigned int FileReader::getChannelCount() {
	return info.channels;
}

unsigned int FileReader::getFrameCount() {
	return (unsigned int)info.frames;
}

unsigned int FileReader::getSampleCount() {
	return getChannelCount()*getFrameCount();
}

unsigned int FileReader::read(unsigned int frames, float* buffer) {
	if(handle == NULL) return 0;
	return (unsigned int)sf_readf_float(handle, buffer, frames);
}

unsigned int FileReader::readAll(float* buffer) {
	return read(getFrameCount(), buffer);
}

int64_t FileReader::seek(unsigned int frame) {
	if(handle == NULL) return -1;
	if(frame >= getFrameCount()) frame = getFrameCount()-1;
	int64_t result = sf_seek(handle, frame, SEEK_SET);
	return result;
}

}
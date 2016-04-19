/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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

void FileReader::close() {
	if(handle != NULL)  {
		sf_close(handle);
		handle = NULL;
		info = {0};
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
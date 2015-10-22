/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
//This has to be first for the macro.
#ifdef WIN32
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/libaudioverse.h>
#include <sndfile.h>
#include <libaudioverse/private/error.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/utf8.hpp>

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

}
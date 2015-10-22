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
#include <libaudioverse/private/error.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/utf8.hpp>
#include <string>

namespace libaudioverse_implementation {

FileWriter::~FileWriter() {
	if(handle) close(); //make sure the file gets closed behind us.
}

void FileWriter::open(const char* path, int sr, int channels) {
	if(handle) close();
	if(sr<= 0) ERROR(Lav_ERROR_RANGE, "sr must be positive.");
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be positive.");
	int length = strlen(path);
	if(length < 4) ERROR(Lav_ERROR_FILE, "File must have a 3-character extension.");
	if(path[length-4] != '.') ERROR(Lav_ERROR_FILE, "File must have a 3-letter extension.");
	//convert to a C++ string for sanity.
	std::string extension(path+length-3);
	if(extension =="wav") info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	else if(extension == "ogg") info.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
	else ERROR(Lav_ERROR_FILE, "Cannot handle extension"+extension);
	info.samplerate =sr;
	info.channels=channels;
	#ifdef WIN32
	std::string p(path);
	auto wp = utf8ToWide(path);
	handle = sf_wchar_open(wp.c_str(), SFM_WRITE, &info);
	#else
	handle = sf_open(path, SFM_WRITE, &info);
	#endif
	if(handle == nullptr) {
		ERROR(Lav_ERROR_FILE, std::string("Libsndfile failed to open ")+path+" for writing.");
	}
}

void FileWriter::close() {
	if(handle != NULL)  {
		sf_close(handle);
		handle = NULL;
		info = {0};
	} else {
		ERROR(Lav_ERROR_FILE, "Attempt to close file before opening.");
	}
}

float FileWriter::getSr() {
	return (float)info.samplerate;
}

unsigned int FileWriter::getChannelCount() {
	return info.channels;
}

unsigned int FileWriter::write(int frames, float* data) {
	if(handle == nullptr) ERROR(Lav_ERROR_FILE, "No open file.");
	return sf_writef_float(handle, data, frames);
}

}
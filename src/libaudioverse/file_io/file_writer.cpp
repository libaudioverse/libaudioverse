/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/libaudioverse.h>
#include <sndfile.h>
#include <libaudioverse/private/errors.hpp>
#include <string>

namespace libaudioverse_implementation {

FileWriter::~FileWriter() {
	if(handle) close(); //make sure the file gets closed behind us.
}

void FileWriter::open(const char* path, int sr, int channels) {
	if(handle) close();
	if(sr<= 0 || channels <= 0) throw LavErrorException(Lav_ERROR_RANGE);
	int length = strlen(path);
	if(length < 4) throw LavErrorException(Lav_ERROR_FILE); //we don't have enough characters to have an extension.
	if(path[length-4] != '.') throw LavErrorException(Lav_ERROR_FILE); //File name does not have 3-letter extension.
	//convert to a C++ string for sanity.
	std::string extension(path+length-3);
	if(extension =="wav") info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	else if(extension == "ogg") info.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
	else throw LavErrorException(Lav_ERROR_FILE);
	info.samplerate =sr;
	info.channels=channels;
	handle = sf_open(path, SFM_WRITE, &info);
	if(handle == nullptr) {
		throw LavErrorException(Lav_ERROR_FILE);
	}
}

void FileWriter::close() {
	if(handle != NULL)  {
		sf_close(handle);
		handle = NULL;
		info = {0};
	} else {
		throw LavErrorException(Lav_ERROR_FILE);
	}
}

float FileWriter::getSr() {
	return (float)info.samplerate;
}

unsigned int FileWriter::getChannelCount() {
	return info.channels;
}

unsigned int FileWriter::write(int frames, float* data) {
	if(handle == nullptr) throw LavErrorException(Lav_ERROR_FILE);
	return sf_writef_float(handle, data, frames);
}

}
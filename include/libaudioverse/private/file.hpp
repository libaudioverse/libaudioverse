/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include <sndfile.h>
#include <inttypes.h>
#include <cstddef>
#include <memory>

namespace libaudioverse_implementation {

class LibsndfileBufferWrapper {
	public:
	LibsndfileBufferWrapper(char* _buffer, int64_t _len);
	//The handle is not safe to use once this object has died.
	void open(int mode, SNDFILE** handle, SF_INFO *info);
	char* buffer = nullptr;
	int64_t len = 0;
	int64_t pos = 0;
	SF_VIRTUAL_IO context; //Libsndfile callbacks.
};

/**A completely stand-alone wrapper around Libsndfile for file reading: implements both a  streaming and non-streaming interface.

Note that there's no getPosition. Libsndfile doesn't provide it.
Classes who need that info need to do the book-keeping to get it themselves.*/
class FileReader: std::enable_shared_from_this<FileReader>  {
	public:
	FileReader(): info() {} //vc++ crashes if we try to do this the c++11 way.
	~FileReader();
	void open(const char* path);
	//This FileReader does not take ownership of the buffer.
	void openFromBuffer(char* buffer, int64_t len);
	void close();
	float getSr();
	unsigned int getChannelCount();
	unsigned int getFrameCount();
	unsigned int getSampleCount();
	unsigned int readAll(float* buffer);
	unsigned int read(unsigned int frames, float* buffer);
	//Clamps if out of range. Returns the count of the new frame or -1 on error.
	int64_t seek(unsigned int frame);
	protected:
	SNDFILE* handle = nullptr;
	SF_INFO info;
	LibsndfileBufferWrapper* buffer_wrapper = nullptr;
};

class FileWriter {
	public:
	FileWriter(): info() {}
	~FileWriter();
	void open(const char* path, int sr, int channels);
	void close();
	float getSr();
	unsigned int getChannelCount();
	//write frames of interleaved float data.
	unsigned int write(int frames, float* data);
	private:
	SNDFILE *handle = nullptr;
	SF_INFO info;
};

}
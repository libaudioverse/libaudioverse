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
#include <algorithm>
#include <cstddef>

//This is coming in from windows.h.
#undef min

namespace libaudioverse_implementation {

sf_count_t get_filelen_cb(void* u) {
	auto c = (LibsndfileBufferWrapper*)u;
	return (sf_count_t)c->len;
}

sf_count_t seek_cb(sf_count_t offset, int wence, void* u) {
	auto c = (LibsndfileBufferWrapper*)u;
	if(wence == SEEK_CUR) {
		c->pos = c->pos+offset;
	}
	else if(wence == SEEK_SET) {
		c->pos = offset;
	}
	else if(wence == SEEK_END) {
		c->pos = c->len+offset;
	}
	return (sf_count_t)c->pos;
}

sf_count_t read_cb(void* ptrv, sf_count_t count, void* u) {
	auto c = (LibsndfileBufferWrapper*)u;
	auto ptr = (char*)ptrv;
	int64_t copyable = std::min(c->len-c->pos, (int64_t)count);
	std::copy(c->buffer+c->pos, c->buffer+c->pos+copyable, ptr);
	c->pos += copyable;
	return (sf_count_t)copyable;
}

sf_count_t write_cb(const void* ptrv, sf_count_t count, void* u) {
	auto c = (LibsndfileBufferWrapper*)u;
	auto ptr = (const char*)ptrv;
	std::size_t copyable = std::min(c->len-c->pos, (int64_t)count);
	std::copy(ptr, ptr+copyable, c->buffer+c->pos);
	c->pos += copyable;
	return (sf_count_t)copyable;
}

sf_count_t tell_cb(void* u) {
	auto c = (LibsndfileBufferWrapper*)u;
	return (sf_count_t)c->pos;
}

LibsndfileBufferWrapper::LibsndfileBufferWrapper(char* _buffer, int64_t _len): buffer(_buffer), len(_len) {
	context.get_filelen = get_filelen_cb;
	context.seek = seek_cb;
	context.read = read_cb;
	context.write = write_cb;
	context.tell = tell_cb;
}

void LibsndfileBufferWrapper::open(int mode, SNDFILE** handle, SF_INFO* info) {
	*handle = sf_open_virtual(&(this->context), mode, info, (void*)this);
}

}
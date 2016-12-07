/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/private/buffer.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/error.hpp>
#include <libaudioverse/private/macros.hpp>
#include <algorithm>
#include <atomic>


namespace libaudioverse_implementation {

Buffer::Buffer(std::shared_ptr<Server> server): ExternalObject(Lav_OBJTYPE_BUFFER) {
	this->server = server;
}

std::shared_ptr<Buffer> createBuffer(std::shared_ptr<Server>server) {
	return std::shared_ptr<Buffer>(new Buffer(server), ObjectDeleter(server));
}

Buffer::~Buffer() {
	if(data) delete[] data;
}

std::shared_ptr<Server> Buffer::getServer() {
	return server;
}

int Buffer::getLength() {
	return frames;
}

double Buffer::getDuration() {
	return frames / server->getSr();
}

int Buffer::getChannels() {
	return channels;
}

void Buffer::loadFromArray(int sr, int channels, int frames, float* inputData) {
	int serverSr= (int)server->getSr();
	if(data) {
		delete[] data;
		data = nullptr;
	}
	staticResamplerKernel(sr, serverSr, channels, frames, inputData, &(this->frames), &data);
	if(data==nullptr) ERROR(Lav_ERROR_MEMORY);
	this->channels = channels;
	if(this->channels == 1) return; //It's already uninterleaved.
	//Uninterleave the data and delete the old one.
	float* newData = new float[this->channels*this->frames];
	for(int ch = 0; ch < this->channels; ch++) {
		for(int i = 0; i < this->frames; i++) {
			newData[ch*this->frames+i] = data[this->channels*i+ch];
		}
	}
	delete[] data;
	data = newData;
}

float Buffer::getSample(int frame, int channel) {
	return data[frames*channel+frame];
}

float* Buffer::getPointer(int frame, int channel) {
	return data+channel*frames+frame;
}

void Buffer::normalize() {
	float min = *std::min_element(data, data+channels*frames);
	float max = *std::max_element(data, data+channels*frames);
	float normfactor = std::max(fabs(min), fabs(max));
	normfactor = 1.0f/normfactor;
	scalarMultiplicationKernel(channels*frames, normfactor, data, data);
}

void Buffer::lock() {
	server->lock();
}

void Buffer::unlock() {
	server->unlock();
}

void Buffer::incrementUseCount() {
	use_count.fetch_add(1);
}

void Buffer::decrementUseCount() {
	use_count.fetch_add(-1);
}

void Buffer::throwIfInUse() {
	if(use_count.load()) {
		ERROR(Lav_ERROR_BUFFER_IN_USE, "You cannot modify buffers while something is using their data.");
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createBuffer(LavHandle serverHandle, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	*destination = outgoingObject(createBuffer(server));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferGetServer(LavHandle handle, LavHandle* destination) {
	PUB_BEGIN
	auto b= incomingObject<Buffer>(handle);
	*destination = outgoingObject(b->getServer());
	PUB_END
}

void loadFromFileReader(Buffer& buff, FileReader& fr) {
	float* data = allocArray<float>(fr.getSampleCount());
	fr.readAll(data);
	{
		LOCK(buff);
		buff.throwIfInUse();
		buff.loadFromArray(fr.getSr(), fr.getChannelCount(), fr.getSampleCount()/fr.getChannelCount(), data);
	}
	freeArray(data);
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromFile(LavHandle bufferHandle, const char* path) {
	PUB_BEGIN
	auto buff =incomingObject<Buffer>(bufferHandle);
	FileReader f{};
	f.open(path);
	loadFromFileReader(*buff, f);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromArray(LavHandle bufferHandle, int sr, int channels, int frames, float* data) {
	PUB_BEGIN
	auto buff=incomingObject<Buffer>(bufferHandle);
	LOCK(*buff);
	buff->throwIfInUse();
	buff->loadFromArray(sr, channels, frames, data);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferDecodeFromArray(LavHandle bufferHandle, char* data, int datalen) {
	PUB_BEGIN
	FileReader fr{};
	auto buff = incomingObject<Buffer>(bufferHandle);
	fr.openFromBuffer(data, datalen);
	loadFromFileReader(*buff, fr);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferNormalize(LavHandle bufferHandle) {
	PUB_BEGIN
	auto b = incomingObject<Buffer>(bufferHandle);
	LOCK(*b);
	b->throwIfInUse();
	b->normalize();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferGetDuration(LavHandle bufferHandle, float* destination) {
	PUB_BEGIN
	auto b = incomingObject<Buffer>(bufferHandle);
	LOCK(*b);
	*destination = b->getDuration();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferGetLengthInSamples(LavHandle bufferHandle, int* destination) {
	PUB_BEGIN
	auto b = incomingObject<Buffer>(bufferHandle);
	LOCK(*b);
	*destination = b->getLength();
	PUB_END
}

}
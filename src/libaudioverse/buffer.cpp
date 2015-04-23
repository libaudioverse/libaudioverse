/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/private/buffer.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/errors.hpp>
#include <libaudioverse/private/macros.hpp>
#include <algorithm>

LavBuffer::LavBuffer(std::shared_ptr<LavSimulation> simulation): LavExternalObject(Lav_OBJTYPE_BUFFER) {
	this->simulation = simulation;
}

std::shared_ptr<LavBuffer> createBuffer(std::shared_ptr<LavSimulation>simulation) {
	return std::shared_ptr<LavBuffer>(new LavBuffer(simulation), LavObjectDeleter(simulation));
}

LavBuffer::~LavBuffer() {
	if(data) LavFreeArray(data);
}

std::shared_ptr<LavSimulation> LavBuffer::getSimulation() {
	return simulation;
}

int LavBuffer::getLength() {
	return frames;
}

double LavBuffer::getDuration() {
	return frames / simulation->getSr();
}

int LavBuffer::getChannels() {
	return channels;
}

void LavBuffer::loadFromArray(int sr, int channels, int frames, float* inputData) {
	int simulationSr= (int)simulation->getSr();
	staticResamplerKernel(sr, simulationSr, channels, frames, inputData, &(this->frames), &data);
	if(data==nullptr) throw LavErrorException(Lav_ERROR_MEMORY);
	data_end=data+this->frames*channels;
	this->channels = channels;
}

int LavBuffer::writeData(int startFrame, int channels, int frames, float** outputs) {
	//we know that writeChannel always returns the same value for all channels and the same startFrame and frames.
	int count = 0;
	for(int i=0; i < channels; i++) {
		count = writeChannel(startFrame, i, channels, frames, outputs[i]);
		if(count == 0) break;
	}
	return count;
}

int LavBuffer::writeChannel(int startFrame, int channel, int maxChannels, int frames, float* dest) {
	if(channel >=maxChannels) return 0; //bad, very bad.
	const float* matrix= simulation->getMixingMatrix(this->channels, maxChannels);
	float* readFrom =data+startFrame*this->channels;
	int willWrite = this->frames-startFrame;
	if(willWrite <= 0) return 0;
	if(willWrite > frames) willWrite = frames;
	if(channel>= this->channels && matrix== nullptr) return willWrite; //we wrote according to the drop channels rule and our destination is supposed to be zeroed.
	else if(matrix) {
		for(int i=0; i < willWrite; i++) {
			for(int j = 0; j < this->channels; j++) {
				dest[i] += readFrom[i*this->channels+j]*matrix[channel*this->channels+j];
			}
		}
		return willWrite;
	}
	else {
		for(int i=0; i < willWrite; i++) dest[i] = readFrom[i*this->channels+channel];
		return willWrite;
	}
}

float LavBuffer::getSample(int frame, int channel) {
	return data[frame*channels+channel];
}

float LavBuffer::getSampleWithMixingMatrix(int frame, int channel, int maxChannels) {
	auto mat =simulation->getMixingMatrix(channels, maxChannels);
	if(mat) {
		float res= 0.0;
		for(int i=0; i < channels; i++) res += data[frame*channels+i]*mat[channels*channel+i];
		return res;
	}
	else if(channel < channels) return data[frame*channels+channel];
	else return 0.0;
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createBuffer(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject(createBuffer(simulation));
	PUB_END
}
Lav_PUBLIC_FUNCTION LavError Lav_bufferGetSimulation(LavHandle handle, LavHandle* destination) {
	PUB_BEGIN
	auto b= incomingObject<LavBuffer>(handle);
	*destination = outgoingObject(b->getSimulation());
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromFile(LavHandle bufferHandle, const char* path) {
	PUB_BEGIN
	auto buff =incomingObject<LavBuffer>(bufferHandle);
	LavFileReader f{};
	f.open(path);
	float* data = LavAllocArray<float>(f.getSampleCount());
	f.readAll(data);
	{
		LOCK(*buff);
		buff->loadFromArray(f.getSr(), f.getChannelCount(), f.getSampleCount()/f.getChannelCount(), data);
	}
	LavFreeArray(data);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromArray(LavHandle bufferHandle, int sr, int channels, int frames, float* data) {
	PUB_BEGIN
	auto buff=incomingObject<LavBuffer>(bufferHandle);
	LOCK(*buff);
	buff->loadFromArray(sr, channels, frames, data);
	PUB_END
}

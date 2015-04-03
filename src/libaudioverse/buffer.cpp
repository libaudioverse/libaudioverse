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

LavBuffer::LavBuffer(std::shared_ptr<LavSimulation> simulation) {
	this->simulation = simulation;
}

std::shared_ptr<LavBuffer> createBuffer(std::shared_ptr<LavSimulation>simulation) {
	return std::shared_ptr<LavBuffer>(new LavBuffer(simulation), LavObjectDeleter);
}

LavBuffer::~LavBuffer() {
	if(data) LavFreeFloatArray(data);
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
	int count =0;
	const float* matrix = simulation->getMixingMatrix(this->channels, channels);
	const float* dataOffset=data+(this->channels*startFrame);
	//request past end check.
	if(dataOffset >= data_end) return 0;
	if(channels==this->channels || matrix==nullptr) { //just a copy with a twist.
		for(int i =0; i < channels; i++) {
			for(int j=0; j < channels; j++) {
				if(j < this->channels) outputs[j][i] = dataOffset[j];
				else outputs[j][i] =0.0f;
			}
			dataOffset+=this->channels;
			count +=1;
			if(dataOffset>= data_end) break;
		}
	}
	else { //we have a mixing matrix and the channels are different.
		//We have to do this by hand because we're un-interleaving things at the same time we apply the mixing matrix.
		for(int i=0; i < frames; i++) {
			for(int j= 0; j < channels; j++) {
				//the jth row of the mixing matrix multiplied by this frame is the output value for the jth output.
				outputs[j][i] = 0.0f;
				for(int k=0; k < this->channels; k++) {
					outputs[j][i] += dataOffset[j]*matrix[j*this->channels+k];
				}
			}
			count++;
			dataOffset+= this->channels;
			if(dataOffset >= data_end) break;
		}
	}
	return count;
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createBuffer(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject(createBuffer(simulation));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromFile(LavHandle bufferHandle, const char* path) {
	PUB_BEGIN
	auto buff =incomingObject<LavBuffer>(bufferHandle);
	LavFileReader f{};
	f.open(path);
	float* data = LavAllocFloatArray(f.getSampleCount());
	f.readAll(data);
	{
		LOCK(*buff);
		buff->loadFromArray(f.getSr(), f.getChannelCount(), f.getSampleCount()/f.getChannelCount(), data);
	}
	LavFreeFloatArray(data);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromArray(LavHandle bufferHandle, int sr, int channels, int frames, float* data) {
	PUB_BEGIN
	auto buff=incomingObject<LavBuffer>(bufferHandle);
	LOCK(*buff);
	buff->loadFromArray(sr, channels, frames, data);
	PUB_END
}


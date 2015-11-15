/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/fft_convolver.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/implementations/convolvers.hpp>
#include <string>

namespace libaudioverse_implementation {

FftConvolverNode::FftConvolverNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_FFT_CONVOLVER_NODE, simulation, channels, channels) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	appendInputConnection(0, channels);
	this->channels=channels;
	appendOutputConnection(0, channels);
	convolvers=new FftConvolver*[channels]();
	for(int i= 0; i < channels; i++) convolvers[i] = new FftConvolver(simulation->getBlockSize());
}

std::shared_ptr<Node> createFftConvolverNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<FftConvolverNode>(simulation, channels);
}

FftConvolverNode::~FftConvolverNode() {
	for(int i = 0; i < channels; i++) delete convolvers[i];
	delete[] convolvers;
}

void FftConvolverNode::process() {
	for(int i= 0; i < channels; i++) {
		convolvers[i]->convolve(input_buffers[i], output_buffers[i]);
	}
}

void FftConvolverNode::setResponse(int channel, int length, float* response) {
	if(channel >= channels || channel < 0) ERROR(Lav_ERROR_RANGE, "Channel out of range.");
	if(length < 1) ERROR(Lav_ERROR_RANGE, "Response must be at least one sample.");
	convolvers[channel]->setResponse(length, response);
	convolvers[channel]->reset();
}

void FftConvolverNode::setResponseFromFile(std::string path, int fileChannel, int convolverChannel) {
	if(convolverChannel < 0 || convolverChannel >= channels) ERROR(Lav_ERROR_RANGE, "Channel out of range.");
	if(fileChannel < 0) ERROR(Lav_ERROR_RANGE, "File channel must be positive.");
	FileReader reader{};
	reader.open(path.c_str());
	if(fileChannel >= reader.getChannelCount()) ERROR(Lav_ERROR_RANGE, "Channel greater than channels in file.");
	unsigned int bufferSize= reader.getSampleCount();
	float* tmp=allocArray<float>(bufferSize);
	reader.readAll(tmp);
	//This is a strange trick.  Because we only care about the specified channel, we can kill the others.
	//Consequently, we copy the channel of interest to the beginning of the buffer.
	for(int i = 0; i < reader.getFrameCount(); i++) tmp[i] = tmp[i*reader.getChannelCount()+fileChannel];
	//Resample if needed.
	float* resampledTmp;
	int resampledTmpLength;
	staticResamplerKernel(reader.getSr(), simulation->getSr(), 1, reader.getFrameCount(), tmp, &resampledTmpLength, &resampledTmp);
	//Finally, set the specified convolver.
	setResponse(convolverChannel, resampledTmpLength, resampledTmp);
	freeArray(tmp);
	delete[] resampledTmp;
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createFftConvolverNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createFftConvolverNode(simulation, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_fftConvolverNodeSetResponse(LavHandle nodeHandle, int channel, int length, float* response) {
	PUB_BEGIN
	auto n = incomingObject<FftConvolverNode>(nodeHandle);
	LOCK(*n);
	n->setResponse(channel, length, response);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_fftConvolverNodeSetResponseFromFile(LavHandle nodeHandle, const char* path, int fileChannel, int convolverChannel) {
	PUB_BEGIN
	auto n = incomingObject<FftConvolverNode>(nodeHandle);
	LOCK(*n);
	n->setResponseFromFile(path, fileChannel, convolverChannel);
	PUB_END
}

}
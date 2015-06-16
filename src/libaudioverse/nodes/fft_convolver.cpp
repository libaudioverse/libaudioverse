/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/functiontables.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/implementations/convolvers.hpp>
#include <limits>
#include <string>

namespace libaudioverse_implementation {

class FftConvolverNode: public Node {
	public:
	FftConvolverNode(std::shared_ptr<Simulation> simulation, int channels);
	~FftConvolverNode();
	virtual void process();
	void setResponse(int channel, int length, float* response);
	void setResponseFromFile(std::string path, int fileChannel, int convolverChannel);
	int channels;
	FftConvolver **convolvers;
};

FftConvolverNode::FftConvolverNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_FFT_CONVOLVER_NODE, simulation, channels, channels) {
	if(channels < 1) throw LavErrorException(Lav_ERROR_RANGE);
	appendInputConnection(0, channels);
	this->channels=channels;
	appendOutputConnection(0, channels);
	convolvers=new FftConvolver*[channels]();
	for(int i= 0; i < channels; i++) convolvers[i] = new FftConvolver(simulation->getBlockSize());
}

std::shared_ptr<Node> createFftConvolverNode(std::shared_ptr<Simulation> simulation, int channels) {
	std::shared_ptr<FftConvolverNode> retval = std::shared_ptr<FftConvolverNode>(new FftConvolverNode(simulation, channels), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

FftConvolverNode::~FftConvolverNode() {
	for(int i = 0; i < channels; i++) delete convolvers[i];
	delete[] convolvers;
}

void FftConvolverNode::process() {
	for(int i= 0; i < channels; i++) convolvers[i]->convolve(input_buffers[i], output_buffers[i]);
}

void FftConvolverNode::setResponse(int channel, int length, float* response) {
	if(channel >= channels || channel < 0) throw LavErrorException(Lav_ERROR_RANGE);
	if(length < 1) throw LavErrorException(Lav_ERROR_RANGE);
	convolvers[channel]->setResponse(length, response);
}

void FftConvolverNode::setResponseFromFile(std::string path, int fileChannel, int convolverChannel) {
	if(convolverChannel < 0 || convolverChannel >= channels) throw LavErrorException(Lav_ERROR_RANGE);
	if(fileChannel < 0) throw LavErrorException(Lav_ERROR_RANGE);
	FileReader reader{};
	reader.open(path.c_str());
	if(fileChannel >= reader.getChannelCount()) throw LavErrorException(Lav_ERROR_RANGE);
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
	convolvers[convolverChannel]->setResponse(resampledTmpLength, resampledTmp);
	freeArray(tmp);
	freeArray(resampledTmp);
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
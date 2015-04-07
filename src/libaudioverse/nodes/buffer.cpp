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
#include <libaudioverse/private/buffer.hpp>
#include <limits>
#include <vector>

class LavBufferNode: public LavNode {
	public:
	LavBufferNode(std::shared_ptr<LavSimulation> simulation);
	virtual void bufferChanged();
	virtual void positionChanged();
	virtual void process();
	int frame = 0;
	int buffer_length=0;
	double offset=0.0;
};

LavBufferNode::LavBufferNode(std::shared_ptr<LavSimulation> simulation): LavNode(Lav_NODETYPE_BUFFER, simulation, 0, 1) {
	appendOutputConnection(0, 1);
	getProperty(Lav_BUFFER_BUFFER).setPostChangedCallback([&] () {bufferChanged();});
	getProperty(Lav_BUFFER_POSITION).setPostChangedCallback([&] () {positionChanged();});
}

std::shared_ptr<LavNode> createBufferNode(std::shared_ptr<LavSimulation> simulation) {
	printf("Buffer node created\n");
	std::shared_ptr<LavBufferNode> retval = std::shared_ptr<LavBufferNode>(new LavBufferNode(simulation), LavObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void LavBufferNode::bufferChanged() {
	printf("Buffer changed...\n");
	auto buff= getProperty(Lav_BUFFER_BUFFER).getBufferValue();
	double maxPosition= 0.0;
	int newChannels= 0;
	int newBufferLength=0;
	if(buff==nullptr) {
		resize(0, 1);
		getOutputConnection(0)->reconfigure(0, 1);
	}
	else {
		printf("Have a buffer\n");
		newChannels = buff->getChannels() > 0 ? buff->getChannels() : 1;
		resize(0, newChannels);
		getOutputConnection(0)->reconfigure(0, newChannels);
		maxPosition =buff->getDuration();
		newBufferLength=buff->getLength();
	}
	getProperty(Lav_BUFFER_POSITION).setDoubleValue(0.0); //the callback handles changing everything else.
	getProperty(Lav_BUFFER_POSITION).setDoubleRange(0.0, maxPosition);
	buffer_length = newBufferLength;
	printf("%i\n", newBufferLength);
}

void LavBufferNode::positionChanged() {
	if(is_processing) return;
	frame = (int)(getProperty(Lav_BUFFER_POSITION).getDoubleValue()*simulation->getSr());
	offset = 0.0;
}

void LavBufferNode::process() {
	auto buff = getProperty(Lav_BUFFER_BUFFER).getBufferValue();
	if(buff== nullptr) return; //no buffer.
	if(buffer_length== 0) return;
	float delta=getProperty(Lav_BUFFER_PITCH_BEND).getFloatValue();
	bool isLooping = getProperty(Lav_BUFFER_LOOPING).getIntValue() == 1;
	//We do the looping check first so we can break out if we have issues.
	for(int i =0; i < block_size; i++) {
		if(frame >= buffer_length) { //past end.
			getEvent(Lav_BUFFER_END_EVENT).fire();
			if(isLooping == false) break;
			frame= 0;
		}
		for(int chan =0; chan < num_output_buffers; chan++) {
			//We always have as many samples as output channels.
			//This is standard linear interpolation.
			double a = buff->getSample(frame, chan);
			double b;
			if(frame+1 < buffer_length) b = buff->getSample(frame+1, chan); //okay, we have one more sample after this one.
			else if(isLooping) b =buff->getSample(0, chan); //We have a next sample, but it's looped to the beginning.
			else b = 0.0; //no next sample.
			double weight1 = 1-offset;
			double weight2 = offset;
			output_buffers[chan][i] = (float)(weight1*a+weight2*b);
		}
		offset+=delta;
		frame += floor(offset);
		offset = offset-floor(offset);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createBufferNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createBufferNode(simulation);
	*destination = outgoingObject<LavNode>(retval);
	PUB_END
}

/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Reads an entire file into memory and plays it with support for dopler and seeking.*/
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/memory.hpp>
#include <limits>
#include <memory>
#include <math.h>

namespace libaudioverse_implementation {

class FileNode: public Node {
	public:
	FileNode(std::shared_ptr<Simulation> simulation, const char* path, unsigned int channels);
	~FileNode();
	virtual void process();
	void seek(); //property callback.
	protected:
	FileReader file;
	float* buffer = nullptr;
	unsigned int position = 0, frame_count = 0;
	float offset = 0;
	float delta = 0.0f;
	double max_position = 0.0f; //save us querying it.
	bool has_ended = false;
};

//the third parameter is a hint: we need to know how many channels, we only expose objects through the create functions, so the create function can find this out.
//todo: when objects support resizing their inputs and outputs, as they will inevitably support this, rewrite to use that functionality.
FileNode::FileNode(std::shared_ptr<Simulation> simulation, const char* path, unsigned int channels): Node(Lav_OBJTYPE_FILE_NODE, simulation, 0, channels) {
	file.open(path);
	buffer = allocArray<float>(file.getSampleCount());
	file.readAll(buffer);
	delta = file.getSr()/simulation->getSr();
	getProperty(Lav_FILE_POSITION).setPostChangedCallback([this] () {seek();});
	max_position = file.getFrameCount()/(float)file.getSr();
	getProperty(Lav_FILE_POSITION).setDoubleRange(0.0f, max_position);
	frame_count = file.getFrameCount();
	appendOutputConnection(0, channels);
}

FileNode::~FileNode() {
	freeArray(buffer);
}

std::shared_ptr<Node> createFileNode(std::shared_ptr<Simulation> simulation, const char* path) {
	auto f = FileReader();
	f.open(path);
	auto retval = std::shared_ptr<FileNode>(new FileNode(simulation, path, f.getChannelCount()), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void FileNode::seek() {
	if(is_processing) return;
	double pos = getProperty(Lav_FILE_POSITION).getDoubleValue();
	offset = 0.0f;
	position = (unsigned int)(pos*file.getSr());
	has_ended = false;
}

void FileNode::process() {
	bool isLooping = (bool)getProperty(Lav_FILE_LOOPING).getIntValue();
	if(has_ended) {
		if(isLooping) {
			position = 0;
			offset = 0.0f;
			has_ended = false;
		}
		else {
			return;
		}
	}
	bool switch_to_ended = false;
	const float pitch_bend = getProperty(Lav_FILE_PITCH_BEND).getFloatValue();
	for(unsigned int i = 0; i < block_size; i++) {
		if(position >= frame_count) {
			if(isLooping) {
				position = 0;
				offset = 0;
			}
			else {
				for(unsigned int j = 0; j < num_output_buffers; j++) {
					output_buffers[j][i] = 0.0f;
				}
				switch_to_ended = true;
				continue;
			}
		}
		unsigned int samp1 = (unsigned int)position;
		unsigned int samp2 = (unsigned int)position+1;
		if(samp2 >= frame_count) samp2--;
		const float weight1 = 1-offset;
		const float weight2 = offset;
		for(unsigned int j = 0; j < num_output_buffers; j++) {
			const unsigned int ind1 = samp1*num_output_buffers+j;
			const unsigned int ind2 = samp2*num_output_buffers+j;
			output_buffers[j][i] = weight1*buffer[ind1]+weight2*buffer[ind2];
		}		
	offset += delta*pitch_bend;
	position += (unsigned int)offset;
	offset = ringmodf(offset, 1.0f);
	}
	double newpos = ((double)position+offset)/(double)simulation->getSr();
	newpos = fmin(newpos, max_position);
	getProperty(Lav_FILE_POSITION).setDoubleValue(newpos);
	if(switch_to_ended) {
		has_ended = true;
		getEvent(Lav_FILE_END_EVENT).fire();
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createFileNode(LavHandle simulationHandle, const char* path, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createFileNode(simulation, path);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}
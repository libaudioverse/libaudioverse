/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../libaudioverse.h"
#include "properties.hpp"
#include "events.hpp"
#include <map>
#include <memory>
#include <vector>

class LavProperty;

class LavInputDescriptor {
	public:
	LavInputDescriptor(std::shared_ptr<LavNode> p, unsigned int o): parent(p), output(o) {}
	std::weak_ptr<LavNode> parent;
	unsigned int output = 0;
};

/**Things all Libaudioverse nodes have.*/
class LavNode: public std::enable_shared_from_this<LavNode> { //enable_shared_from_this is for event infrastructure.
	public:
	LavNode(int type, std::shared_ptr<LavSimulation> simulation, unsigned int numInputBuffers, unsigned int numOutputBuffers);
	virtual ~LavNode();

	virtual int getType();

	virtual unsigned int getOutputBufferCount();
	//Note that this isn't shared ptr.  The output pointers for a node are managed by the node itself and we need to be able to allocate/deallocate them for SSE, as well as work with arrays.  Don't hold on to output pointers.
	float** getOutputBufferArray();

	//equivalent to reading lav_NODE_STATE.
	virtual int getState();

	//do not override. Handles the processing protocol (updating some globals and calling process) if needed for this tick, otherwise does nothing.
	void tick();
	//override this one instead. Default implementation merely zeros the outputs.
	virtual void process();
	//zero the output buffers.
	virtual void zeroOutputBuffers();
	//Does some cleanup and the like.
	//This is also an override point for subclasses that may need to do cleanup periodically in order to remain performant; in that case, they *must* call the base. Or else.
	virtual void doMaintenance();
	//this is called at some point in the processing logic which is guaranteed to be before this node's parents are processed and after the device is locked.
	//additionally, a parent will have its willProcessParents called after this node.
	//that is, nothing else will touch this node but the mixer thread, and the next thing to be called (at some point in future) is willProcess.
	//the default does nothing.
	virtual void willProcessParents();

	virtual std::shared_ptr<LavSimulation> getSimulation();
	virtual LavProperty& getProperty(int slot);

	//event helper methods.
	LavEvent& getEvent(int which);

	//meet the lockable concept.
	//Warning: these aren't virtual because they're just so that our macro works; all locking still forwards to devices.
	void lock();
	void unlock();

	//Override hook for resetting.
	virtual void reset();
	protected:
	//this should definitely be protected, and should never be touched by anything that's not a subclass.
	virtual void resize(int newInputCount, int newOutputCount);
	std::shared_ptr<LavSimulation> simulation = nullptr;
	std::map<int, LavProperty> properties;
	std::map<int, LavEvent> events;
	std::vector<float*> input_buffers;
	std::vector<LavInputDescriptor> input_descriptors;
	std::vector<float*> output_buffers;
	bool is_processing = false, is_suspended = false;
	int type = Lav_NODETYPE_GENERIC;
	int num_input_buffers = 0, num_output_buffers = 0, block_size = 0;
	//used to make no-op state changes free.
	int prev_state = Lav_NODESTATE_PLAYING;
	int last_processed = -1; //-1 so that it's not equal to the simulation's tick counter, which starts at 0.

	//we are never allowed to copy.
	LavNode(const LavNode&) = delete;
	LavNode& operator=(const LavNode&) = delete;
};

//needed for things that wish to encapsulate and manage nodes that the public API isn't supposed to see.
class LavSubgraphNode: public LavNode {
	public:
	LavSubgraphNode(int type, std::shared_ptr<LavSimulation> simulation);

	//todo:rewrite the overrides here to make subgraphs work again once new connections are fully online.
	virtual void configureSubgraph(std::shared_ptr<LavNode> input, std::shared_ptr<LavNode> output);
	protected:
	std::shared_ptr<LavNode> subgraph_input, subgraph_output;
};

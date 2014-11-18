/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_properties.hpp"
#include "private_events.hpp"
#include <map>
#include <memory>
#include <vector>

class LavProperty;

class LavInputDescriptor {
	public:
	LavInputDescriptor(std::shared_ptr<LavObject> p, unsigned int o): parent(p), output(o) {}
	std::weak_ptr<LavObject> parent;
	unsigned int output = 0;
};

/**Things all Libaudioverse objects have.*/
class LavObject: public std::enable_shared_from_this<LavObject> { //enable_shared_from_this is for event infrastructure.
	public:
	LavObject(int type, std::shared_ptr<LavSimulation> simulation, unsigned int numInputs, unsigned int numOutputs);
	virtual ~LavObject();

	virtual void computeInputBuffers();//update what we point to due to parent changes.
	virtual int getType();
	//The private version of inputs.
	//For most objects, inputs are parents.  There are a few special cases, mostly involving subgraphs.
	//This has a semantic meaning to the graph processor, but not necessarily to use code.  See setInput and getInput for the external versions.
	//parents are dependency links.  Inputs are user-facing dependency links intended to hide library-internal objects where necessary.
	virtual void setParent(unsigned int input, std::shared_ptr<LavObject> parent, unsigned int parentOutput);
	virtual std::shared_ptr<LavObject> getParentObject(unsigned int slot);
	virtual unsigned int getParentOutput(unsigned int slot);
	virtual unsigned int getParentCount();

	//these are what is exposed to the public user.
	//Most of them (including default implementations) forward to a get/set parent function.
	virtual void setInput(unsigned int input, std::shared_ptr<LavObject> object, unsigned int output);
	virtual std::shared_ptr<LavObject> getInputObject(unsigned int input);
	virtual unsigned int getInputOutput(unsigned int input);
	virtual unsigned int getInputCount();

	virtual unsigned int getOutputCount();
	//Note that this isn't shared ptr.  The output pointers for an object are managed by the object itself and we need to be able to allocate/deallocate them for SSE, as well as work with arrays.  Don't hold on to output pointers.
	virtual float* getOutputPointer(unsigned int output);
	//this is guaranteed to be written in terms of getOutputPointer, so subclasses can override the former:
	virtual void getOutputPointers(float** dest);

	//equivalent to reading lav_OBJECT_STATE.
	virtual int getState();

	//do not override. Handles the processing protocol: updating some globals and calling process.
	void doProcessProtocol();
	//override this one instead.
	virtual void process();

	//this is called at some point in the processing logic which is guaranteed to be before this object's parents are processed and after the device is locked.
	//additionally, a parent will have its willProcessParents called after this object.
	//that is, nothing else will touch this object but the mixer thread, and the next thing to be called (at some point in future) is willProcess.
	//the default does nothing.
	virtual void willProcessParents();

	virtual std::shared_ptr<LavSimulation> getSimulation();
	virtual LavProperty& getProperty(int slot);
	virtual std::vector<int> getStaticPropertyIndices();

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
	virtual void resize(unsigned int newInputsCount, unsigned int newOutputsCount);
	std::shared_ptr<LavSimulation> simulation = nullptr;
	std::map<int, LavProperty> properties;
	std::map<int, LavEvent> events;
	std::vector<float*> inputs;
	std::vector<LavInputDescriptor> input_descriptors;
	std::vector<float*> outputs;
	bool is_processing = false, is_suspended = false;
	int type = Lav_OBJTYPE_GENERIC;
	unsigned int num_inputs = 0, num_outputs = 0, block_size = 0;

	//we are never allowed to copy.
	LavObject(const LavObject&) = delete;
	LavObject& operator=(const LavObject&) = delete;
};

//needed for things that wish to encapsulate and manage nodes that the public API isn't supposed to see.
class LavSubgraphObject: public LavObject {
	public:
	LavSubgraphObject(int type, std::shared_ptr<LavSimulation> simulation);
	virtual void process();
	//we do no processing and forward onto another object.  Therefore, we do not compute input buffers.
	virtual void computeInputBuffers();
	//these overrides allow us to "step aside" and always say that the output of this subgraph is our parent.
	virtual unsigned int getParentCount();
	virtual std::shared_ptr<LavObject> getParentObject(unsigned int par);
	virtual unsigned int getParentOutput(unsigned int par);
	//and this one crashes us if anything tries to set the parent.
	//the purpose will become clearer once logging is introduced.
	virtual void setParent(unsigned int par, std::shared_ptr<LavObject> obj, unsigned int input);

	virtual unsigned int getOutputCount();
	virtual float* getOutputPointer(unsigned int output);
	virtual void setInput(unsigned int input, std::shared_ptr<LavObject> object, unsigned int output);
	virtual std::shared_ptr<LavObject> getInputObject(unsigned int input);
	virtual unsigned int getInputOutput(unsigned int input);
	virtual unsigned int getInputCount();
	virtual void configureSubgraph(std::shared_ptr<LavObject> input, std::shared_ptr<LavObject> output);
	protected:
	std::shared_ptr<LavObject> subgraph_input, subgraph_output;
};

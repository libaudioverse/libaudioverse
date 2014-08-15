/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_properties.hpp"
#include "private_callbacks.hpp"
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
class LavObject: public std::enable_shared_from_this<LavObject> { //enable_shared_from_this is for callback infrastructure.
	public:
	LavObject(int type, std::shared_ptr<LavDevice> device, unsigned int numInputs, unsigned int numOutputs);
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
	virtual void getOutputPointers(float** dest);

	/**This requires explanation.
The graph algorithm checks all objects it sees.  An object marked to always process will always process, but this is an internal detail of the object.
An object marked as suspended will not process.  In addition, the graph algorithm shall ignore its parents.
Consider an hrtf node, taking 22579200 mathematical operations plus loop overhead and a memory copy.  If it is known in some topology that the hrtf node will be receiving silent input, you can suspend it; if you wish a parent to still advance, you may mark the parent as always needing processing.*/
	virtual bool isSuspended();
	virtual void suspend();
	virtual void unsuspend();

	//these three methods are all involved in the processing logic: willProcess is called immediately before and didProcess immediately after the actual process method.
	//this is a strong guarantee: no other operation shall be performed on this object between these three calls.
	//This one does three things:
	//1. Toggle on is_processing;
	//2. Call computeInputBuffers.
	//3. Update the convenience variables num_inputs and num_outputs to be the sizes of the input and output vectors.
	virtual void willProcess();
	virtual void process();
	virtual void didProcess();

	//this is called at some point in the processing logic which is guaranteed to be before this object's parents are processed and after the device is locked.
	//additionally, a parent will have its willProcessParents called after this object.
	//that is, nothing else will touch this object but the mixer thread, and the next thing to be called (at some point in future) is willProcess.
	//the default does nothing.
	virtual void willProcessParents();

	virtual std::shared_ptr<LavDevice> getDevice();
	virtual LavProperty& getProperty(int slot);
	virtual std::vector<int> getStaticPropertyIndices();

	//callback helper methods.
	LavCallback& getCallback(int which);

	//meet the lockable concept.
	//Warning: these aren't virtual because they're just so that our macro works; all locking still forwards to devices.
	void lock();
	void unlock();

	protected:
	//this should definitely be protected, and should never be touched by anything that's not a subclass.
	virtual void resize(unsigned int newInputsCount, unsigned int newOutputsCount);
	std::shared_ptr<LavDevice> device = nullptr;
	std::map<int, LavProperty> properties;
	std::map<int, LavCallback> callbacks;
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
	LavSubgraphObject(int type, std::shared_ptr<LavDevice> device, unsigned int numChannels);
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
	virtual void getOutputPointers(float** dest);
	virtual void setInput(unsigned int input, std::shared_ptr<LavObject> object, unsigned int output);
	virtual std::shared_ptr<LavObject> getInputObject(unsigned int input);
	virtual unsigned int getInputOutput(unsigned int input);
	virtual unsigned int getInputCount();
	virtual void configureSubgraph(std::shared_ptr<LavObject> input, std::shared_ptr<LavObject> output);
	protected:
	std::shared_ptr<LavObject> subgraph_input, subgraph_output;
};

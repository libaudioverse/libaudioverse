/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include <map>
#include <memory>

class LavProperty;

class LavInputDescriptor {
	public:
	LavObject* parent = nullptr;
	unsigned int output = 0;
};

/**Things all Libaudioverse objects have.*/
class LavObject: std::enable_shared_from_this<LavObject> {
	public:
	LavObject(LavDevice* device, unsigned int numInputs, unsigned int numOutputs);
	virtual ~LavObject() = default;

	virtual void computeInputBuffers();//update what we point to due to parent changes.
	virtual void setParent(unsigned int input, LavObject* parent, unsigned int parentOutput);
	virtual LavObject* getParentObject(unsigned int slot);
	virtual unsigned int getParentOutput(unsigned int slot);
	virtual unsigned int getInputCount();
	virtual unsigned int getOutputCount();
	virtual void getOutputPointers(float** dest);
	virtual void clearParent(unsigned int slot);

	/**This requires explanation.
The graph algorithm checks all objects it sees.  An object marked to always process will always process, but this is an internal detail of the object.
An object marked as suspended will not process.  In addition, the graph algorithm shall ignore its parents.
Consider an hrtf node, taking 22579200 mathematical operations plus loop overhead and a memory copy.  If it is known in some topology that the hrtf node will be receiving silent input, you can suspend it; if you wish a parent to still advance, you may mark the parent as always needing processing.*/
	virtual bool isSuspended();
	virtual void suspend();
	virtual void unsuspend();

	//these three methods are all involved in the processing logic: willProcess is called immediately before and didProcess immediately after the actual process method.
	//this is a strong guarantee: no other operation shall be performed on this object between these three calls.
	//base implementations toggle is_processing, so taht property callbacks can tell who set them-something external to this object or this object itself.
	virtual void willProcess();
	virtual void process();
	virtual void didProcess();

	//this is called at some point in the processing logic which is guaranteed to be before this object's parents are processed and after the device is locked.
	//additionally, a parent will have its willProcessParents called after this object.
	//that is, nothing else will touch this object but the mixer thread, and the next thing to be called (at some point in future) is willProcess.
	//the default does nothing.
	virtual void willProcessParents();

	virtual LavDevice* getDevice();
	virtual LavProperty* getProperty(int slot);

	//meet the lockable concept.
	//Warning: these aren't virtual because they're just so that our macro works; all locking still forwards to devices.
	void lock();
	void unlock();

	protected:
	LavDevice *device = nullptr;
	std::map<int, LavProperty*> properties;
	float** inputs = nullptr;
	LavInputDescriptor *input_descriptors = nullptr;
	float** outputs = nullptr;
	unsigned int num_outputs = 0;
	unsigned int num_inputs = 0;
	bool is_processing = false, is_suspended = false;
	enum Lav_OBJTYPES type = Lav_OBJTYPE_GENERIC;

	//we are never allowed to copy.
	LavObject(const LavObject&) = delete;
	LavObject& operator=(const LavObject&) = delete;
};

//this variant on object is special.  Passes inputs to corresponding outputs.
//needed for things that wish to encapsulate and manage nodes that the public API isn't supposed to see.
class LavPassthroughObject: public LavObject {
	public:
	LavPassthroughObject(LavDevice* device, unsigned int numChannels);
	virtual void process();
};

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
	LavObject() = default; //this is needed due to weirdness with deleting the copy constructor and assignment. For some reason, VS breaks when you do that and not also this.
	virtual ~LavObject() = default;

	virtual void init(LavDevice* device, unsigned int numInputs, unsigned int numOutputs);
	virtual void computeInputBuffers();//update what we point to due to parent changes.
	virtual void setParent(unsigned int input, LavObject* parent, unsigned int parentOutput);
	virtual LavObject* getParentObject(unsigned int slot);
	virtual unsigned int getParentOutput(unsigned int slot);
	virtual unsigned int getInputCount();
	virtual unsigned int getOutputCount();
	virtual void getOutputPointers(float** dest);
	virtual void clearParent(unsigned int slot);
	virtual void process();
	virtual void processor();
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
	int is_processing = 0;
	enum Lav_OBJTYPES type = Lav_OBJTYPE_GENERIC;

	//we are never allowed to copy.
	LavObject(const LavObject&) = delete;
	LavObject& operator=(const LavObject&) = delete;
};


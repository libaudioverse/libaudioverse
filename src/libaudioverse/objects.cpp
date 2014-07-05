/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all objects: linking, allocating, and freeing, as well as parent-child relationships.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <stdlib.h>
#include <string.h>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_metadata.hpp>
#include <algorithm>

float zerobuffer[Lav_MAX_BLOCK_SIZE] = {0}; //this is a shared buffer for the "no parent" case.

void LavObject::computeInputBuffers() {
	//point our inputs either at a zeroed buffer or the output of our parent.
	for(unsigned int i = 0; i < input_descriptors.size(); i++) {
		if(input_descriptors[i].parent != NULL && input_descriptors[i].parent->isSuspended() == false) {
			if(input_descriptors[i].output >= input_descriptors[i].parent->getOutputCount()) { //the parent node no longer has this output, probably due to resizing.
				setParent(i, nullptr, 0); //so we clear this parent.
			}
			inputs[i] = input_descriptors[i].parent->outputs[input_descriptors[i].output];
		}
		else {
			inputs[i] = zerobuffer;
		}
	}
}

LavObject::LavObject(int type, LavDevice* device, unsigned int numInputs, unsigned int numOutputs): type(type) {
	//allocations:
	input_descriptors.resize(numInputs, LavInputDescriptor(nullptr, 0));
	inputs.resize(numInputs, nullptr);
	outputs.resize(numOutputs);
	for(auto i = outputs.begin(); i != outputs.end(); i++) {
		*i = new float[device->getBlockSize()];
	}

	device->associateObject(this);
	this->device = device;
	//request properties from the metadata module.
	properties = makePropertyTable(type);
	computeInputBuffers(); //at the moment, this is going to just make them all 0, but it takes effect once parents are added.
}

LavObject::~LavObject() {
	for(auto i: outputs) {
		delete[] i;
	}
}

void LavObject::willProcess() {
	is_processing = true;
	computeInputBuffers();
	num_inputs = inputs.size();
	num_outputs = outputs.size();
	block_size = device->getBlockSize();
}

/*Default Processing function.*/
void LavObject::process() {
	for(unsigned int i = 0; i < outputs.size(); i++) {
		memset(outputs[i], 0, device->getBlockSize()*sizeof(float));
	}
}

void LavObject::didProcess() {
	is_processing = false;
}

void LavObject::willProcessParents() {
}

int LavObject::getType() {
	return type;
}

bool LavObject::isSuspended() {
	return getProperty(Lav_OBJECT_SUSPENDED).getIntValue() != 0;
}

void LavObject::suspend() {
	getProperty(Lav_OBJECT_SUSPENDED).setIntValue(1);
}

void LavObject::unsuspend() {
	getProperty(Lav_OBJECT_SUSPENDED).setIntValue(0);
}

void LavObject::setParent(unsigned int input, LavObject* parent, unsigned int parentOutput) {
	if(input >= input_descriptors.size()) throw LavErrorException(Lav_ERROR_RANGE);
	if(parent != nullptr && parentOutput >= parent->getOutputCount()) throw LavErrorException(Lav_ERROR_RANGE);
	input_descriptors[input].parent = parent;
	input_descriptors[input].output = parentOutput;
}

LavObject* LavObject::getParentObject(unsigned int input) {
	if(input >= input_descriptors.size()) throw LavErrorException(Lav_ERROR_RANGE);
	return input_descriptors[input].parent;
}

unsigned int LavObject::getParentOutput(unsigned int input) {
	if(input >= input_descriptors.size()) throw LavErrorException(Lav_ERROR_RANGE);
	return input_descriptors[input].output;
}

unsigned int LavObject::getInputCount() {
	return input_descriptors.size();
}

unsigned int LavObject::getOutputCount() {
	return outputs.size();
}

void LavObject::getOutputPointers(float** dest) {
	std::copy(outputs.begin(), outputs.end(), dest);
}

LavDevice* LavObject::getDevice() {
	return device;
}

LavProperty& LavObject::getProperty(int slot) {
	if(properties.count(slot) == 0) throw LavErrorException(Lav_ERROR_RANGE);
	else return properties[slot];
}

std::vector<int> LavObject::getStaticPropertyIndices() {
	std::vector<int> res;
	for(auto i = properties.begin(); i != properties.end(); i++) {
		if(i->first < 0) res.push_back(i->first);
	}
	return res;
}

void LavObject::lock() {
	device->lock();
}

void LavObject::unlock() {
	device->unlock();
}

//protected resize function.
void LavObject::resize(unsigned int newInputCount, unsigned int newOutputCount) {
	//inputs is easy, because we didn't allocate any memory outside of the vector interface.
	inputs.resize(newInputCount, nullptr);
	input_descriptors.resize(newInputCount, LavInputDescriptor(nullptr, 0));
	//but outputs has a special case.
	unsigned int oldOutputCount = outputs.size();
	if(newOutputCount < oldOutputCount) { //we need to free some arrays.
		for(auto i = newOutputCount; i < oldOutputCount; i++) {
			delete[] outputs[i];
		}
	}
	//do the resize.
	outputs.resize(newOutputCount, nullptr);
	if(newOutputCount > oldOutputCount) { //we need to allocate some more arrays.
		for(auto i = oldOutputCount; i < newOutputCount; i++) {
			outputs[i] = new float[device->getBlockSize()];
		}
	}
}

/**Implementation of LavPasssthroughObject.*/

LavPassthroughObject::LavPassthroughObject(int type, LavDevice* device, unsigned int numChannels): LavObject(type, device, numChannels, numChannels) {
}

void LavPassthroughObject::process() {
	for(unsigned int i = 0; i < inputs.size(); i++) {
		std::copy(inputs[i], inputs[i]+device->getBlockSize(), outputs[i]);
	}
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_objectGetType(LavObject* obj, int* destination) {
	PUB_BEGIN
	*destination = obj->getType();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_setParent(LavObject *obj, LavObject*parent, unsigned int outputSlot, unsigned int inputSlot) {
	obj->setParent(inputSlot, parent, outputSlot);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getParent(LavObject *obj, unsigned int slot, LavObject **parent, unsigned int *outputNumber) {
	*parent = obj->getParentObject(slot);
	*outputNumber = obj->getParentOutput(slot);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetInputCount(LavObject* obj, unsigned int* destination) {
	PUB_BEGIN
	LOCK(*obj);
	*destination = obj->getInputCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetOutputCount(LavObject* obj, unsigned int* destination) {
	PUB_BEGIN
	LOCK(*obj);
	*destination = obj->getOutputCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetParentObject(LavObject *obj, unsigned int slot, LavObject** destination) {
	PUB_BEGIN
	LOCK(*(obj->getDevice()));
	*destination = obj->getParentObject(slot);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetParentOutput(LavObject* obj, unsigned int slot, unsigned int* destination) {
	PUB_BEGIN
	LOCK(*obj);
	*destination = obj->getParentOutput(slot);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetParent(LavObject *obj, unsigned int input, LavObject* parent, unsigned int output) {
	PUB_BEGIN
	LOCK(*(obj->getDevice()));
	obj->setParent(input, parent, output);
	PUB_END
}

//this is properties.
//this is here because properties do not "know" about objects and only objects have properties; also, it made properties.cpp ahve to "know" about devices and objects.

//this works for getters and setters to lock the object and set a variable prop to be a pointer-like thing to a property.
#define PROP_PREAMBLE(o, s, t) LOCK(*(o));\
auto &prop = (o)->getProperty((s));\
if(prop.getType() != (t)) {\
throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);\
}

Lav_PUBLIC_FUNCTION LavError Lav_objectResetProperty(LavObject *obj, int slot) {
	PUB_BEGIN
	LOCK(*obj);
	auto prop = obj->getProperty(slot);
	prop.reset();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetIntProperty(LavObject* obj, int slot, int value) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT);
	prop.setIntValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloatProperty(LavObject *obj, int slot, float value) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT);
	prop.setFloatValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetDoubleProperty(LavObject *obj, int slot, double value) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_DOUBLE);
	prop.setDoubleValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetStringProperty(LavObject*obj, int slot, char* value) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_STRING);
	prop.setStringValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloat3Property(LavObject* obj, int slot, float v1, float v2, float v3) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT3);
	prop.setFloat3Value(v1, v2, v3);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloat6Property(LavObject* obj, int slot, float v1, float v2, float v3, float v4, float v5, float v6) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT6);
	prop.setFloat6Value(v1, v2, v3, v4, v5, v6);
	return Lav_ERROR_NONE;
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntProperty(LavObject*obj, int slot, int *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT);
	*destination = prop.getIntValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatProperty(LavObject* obj, int slot, float *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT);
	*destination = prop.getFloatValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetDoubleProperty(LavObject*obj, int slot, double *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_DOUBLE);
	*destination = prop.getDoubleValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetStringProperty(LavObject* obj, int slot, const char** destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_STRING);
	*destination = prop.getStringValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloat3Property(LavObject* obj, int slot, float* v1, float* v2, float* v3) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT3);
	auto val = prop.getFloat3Value();
	*v1 = val[0];
	*v2 = val[1];
	*v3 = val[2];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloat6Property(LavObject* obj, int slot, float* v1, float* v2, float* v3, float* v4, float* v5, float* v6) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT6);
	auto val = prop.getFloat6Value();
	*v1 = val[0];
	*v2 = val[1];
	*v3 = val[2];
	*v4 = val[3];
	*v5 = val[4];
	*v6 = val[5];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntPropertyRange(LavObject* obj, int slot, int* destination_lower, int* destination_upper) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT);
	*destination_lower = prop.getIntMin();
	*destination_upper = prop.getIntMax();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatPropertyRange(LavObject* obj, int slot, float* destination_lower, float* destination_upper) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT);
	*destination_lower = prop.getFloatMin();
	*destination_upper = prop.getFloatMax();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetDoublePropertyRange(LavObject* obj, int slot, double* destination_lower, double* destination_upper) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_DOUBLE);
	*destination_lower = prop.getDoubleMin();
	*destination_upper = prop.getDoubleMax();
	PUB_END
}


Lav_PUBLIC_FUNCTION LavError Lav_objectGetPropertyIndices(LavObject* obj, int** destination) {
	PUB_BEGIN
	LOCK(*obj);
	std::vector<int> indices = obj->getStaticPropertyIndices();
	int* result = new int[indices.size()+1]; //so we can terminate with 0.
	std::copy(indices.begin(), indices.end(), result);
	result[indices.size()] = 0;
	*destination = result;
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetPropertyName(LavObject* obj, int slot, char** destination) {
	PUB_BEGIN
	LOCK(*obj);
	auto prop = obj->getProperty(slot);
	const char* n = prop.getName();
	char* dest = new char[strlen(n)+1]; //+1 for extra NULL.
	strcpy(dest, n);
	*destination = dest;
	PUB_END
}

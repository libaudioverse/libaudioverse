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
#include <algorithm>

float zerobuffer[Lav_MAX_BLOCK_SIZE] = {0}; //this is a shared buffer for the "no parent" case.

void LavObject::computeInputBuffers() {
	//point our inputs either at a zeroed buffer or the output of our parent.
	for(unsigned int i = 0; i < num_inputs; i++) {
		if(input_descriptors[i].parent != NULL && input_descriptors[i].parent->isSuspended() == false) {
			if(input_descriptors[i].output >= input_descriptors[i].parent->getOutputCount()) { //the parent node no longer has this output, probably due to resizing.
				clearParent(i); //so we clear this parent.
			}
			inputs[i] = input_descriptors[i].parent->outputs[input_descriptors[i].output];
		}
		else {
			inputs[i] = zerobuffer;
		}
	}
}

LavObject::LavObject(LavDevice* device, unsigned int numInputs, unsigned int numOutputs) {
	num_inputs = numInputs;
	num_outputs = numOutputs;
	//allocations:
	if(numInputs > 0) {
		input_descriptors = new LavInputDescriptor[numInputs];
		inputs = new float*[numInputs];
	}

	if(numOutputs > 0) {
		outputs = new float*[numOutputs];
		for(unsigned int i = 0; i < num_outputs; i++) {
			outputs[i] = new float[device->getBlockSize()];
		}
	}
	device->associateObject(this);
	this->device = device;

	//set up the suspended property.
	properties[Lav_OBJECT_SUSPENDED] = createIntProperty("suspended", 0, 0, 1);

	computeInputBuffers(); //at the moment, this is going to just make them all 0, but it takes effect once parents are added.
}

void LavObject::willProcess() {
	is_processing = true;
	computeInputBuffers();
}

/*Default Processing function.*/
void LavObject::process() {
	for(unsigned int i = 0; i < num_outputs; i++) {
		memset(outputs[i], 0, device->getBlockSize()*sizeof(float));
	}
}

void LavObject::didProcess() {
	is_processing = false;
}
void LavObject::willProcessParents() {
}

bool LavObject::isSuspended() {
	return properties[Lav_OBJECT_SUSPENDED]->getIntValue() != 0;
}

void LavObject::suspend() {
	properties[Lav_OBJECT_SUSPENDED]->setIntValue(1);
}

void LavObject::unsuspend() {
	properties[Lav_OBJECT_SUSPENDED]->setIntValue(0);
}

void LavObject::setParent(unsigned int input, LavObject* parent, unsigned int parentOutput) {
	if(input >= num_inputs) throw LavErrorException(Lav_ERROR_RANGE);
	if(parentOutput >= parent->getOutputCount()) throw LavErrorException(Lav_ERROR_RANGE);
	input_descriptors[input].parent = parent;
	input_descriptors[input].output = parentOutput;
}

LavObject* LavObject::getParentObject(unsigned int input) {
	if(input >= num_inputs) throw LavErrorException(Lav_ERROR_RANGE);
	return input_descriptors[input].parent;
}

unsigned int LavObject::getParentOutput(unsigned int input) {
	if(input >= num_inputs) throw LavErrorException(Lav_ERROR_RANGE);
	return input_descriptors[input].output;
}

void LavObject::clearParent(unsigned int slot) {
	if(slot >= num_inputs) throw LavErrorException(Lav_ERROR_RANGE);
	input_descriptors[slot].parent = nullptr;
	input_descriptors[slot].output = 0;
}

unsigned int LavObject::getInputCount() {
	return num_inputs;
}

unsigned int LavObject::getOutputCount() {
	return num_outputs;
}

void LavObject::getOutputPointers(float** dest) {
	memcpy(dest, outputs, sizeof(float*)*num_outputs);
}

LavDevice* LavObject::getDevice() {
	return device;
}

LavProperty* LavObject::getProperty(int slot) {
	if(properties.count(slot) == 0) return nullptr;
	else return properties[slot];
}

void LavObject::lock() {
	device->lock();
}

void LavObject::unlock() {
	device->unlock();
}

/**Implementation of LavPasssthroughObject.*/

LavPassthroughObject::LavPassthroughObject(LavDevice* device, unsigned int numChannels): LavObject(device, numChannels, numChannels) {
}

void LavPassthroughObject::process() {
	for(unsigned int i = 0; i < num_inputs; i++) {
		std::copy(inputs[i], inputs[i]+device->getBlockSize(), outputs[i]);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_setParent(LavObject *obj, LavObject*parent, unsigned int outputSlot, unsigned int inputSlot) {
	obj->setParent(inputSlot, parent, outputSlot);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getParent(LavObject *obj, unsigned int slot, LavObject **parent, unsigned int *outputNumber) {
	*parent = obj->getParentObject(slot);
	*outputNumber = obj->getParentOutput(slot);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_clearParent(LavObject *obj, unsigned int slot) {
	obj->clearParent(slot);
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

Lav_PUBLIC_FUNCTION LavError Lav_objectGetParent(LavObject *obj, unsigned int slot, LavObject** parent, unsigned int *outputNumber) {
	PUB_BEGIN
	LOCK(*(obj->getDevice()));
	*parent = obj->getParentObject(slot);
	*outputNumber = obj->getParentOutput(slot);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetParent(LavObject *obj, unsigned int input, LavObject* parent, unsigned int output) {
	PUB_BEGIN
	LOCK(*(obj->getDevice()));
	obj->setParent(input, parent, output);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectClearParent(LavObject *obj, unsigned int slot) {
	PUB_BEGIN
	LOCK(*(obj->getDevice()));
	obj->clearParent(slot);
	PUB_END
}

//this is properties.
//this is here because properties do not "know" about objects and only objects have properties; also, it made properties.cpp ahve to "know" about devices and objects.

//this works for getters and setters to lock the object and set a variable prop to be a pointer-like thing to a property.
#define PROP_PREAMBLE(o, s, t) LOCK(*(o));\
auto prop = (o)->getProperty((s));\
if(prop == nullptr) {\
throw LavErrorException(Lav_ERROR_RANGE);\
}\
if(prop->getType() != (t)) {\
throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);\
}

Lav_PUBLIC_FUNCTION LavError Lav_objectResetProperty(LavObject *obj, int slot) {
	PUB_BEGIN
	LOCK(*obj);
	auto prop = obj->getProperty(slot);
	if(prop == nullptr) throw LavErrorException(Lav_ERROR_RANGE);
	prop->reset();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetIntProperty(LavObject* obj, int slot, int value) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT);
	prop->setIntValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloatProperty(LavObject *obj, int slot, float value) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT);
	prop->setFloatValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetDoubleProperty(LavObject *obj, int slot, double value) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_DOUBLE);
	prop->setDoubleValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetStringProperty(LavObject*obj, int slot, char* value) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_STRING);
	prop->setStringValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloat3Property(LavObject* obj, int slot, float v1, float v2, float v3) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT3);
	prop->setFloat3Value(v1, v2, v3);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloat6Property(LavObject* obj, int slot, float v1, float v2, float v3, float v4, float v5, float v6) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT6);
	prop->setFloat6Value(v1, v2, v3, v4, v5, v6);
	return Lav_ERROR_NONE;
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntProperty(LavObject*obj, int slot, int *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT);
	*destination = prop->getIntValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatProperty(LavObject* obj, int slot, float *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT);
	*destination = prop->getFloatValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetDoubleProperty(LavObject*obj, int slot, double *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_DOUBLE);
	*destination = prop->getDoubleValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetStringProperty(LavObject* obj, int slot, const char** destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_STRING);
	*destination = prop->getStringValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloat3Property(LavObject* obj, int slot, float* v1, float* v2, float* v3) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT3);
	auto val = prop->getFloat3Value();
	*v1 = val[0];
	*v2 = val[1];
	*v3 = val[2];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloat6Property(LavObject* obj, int slot, float* v1, float* v2, float* v3, float* v4, float* v5, float* v6) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT6);
	auto val = prop->getFloat6Value();
	*v1 = val[0];
	*v2 = val[1];
	*v3 = val[2];
	*v4 = val[3];
	*v5 = val[4];
	*v6 = val[5];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntPropertyRange(LavObject* obj, int slot, int* lower, int* upper) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT);
	*lower = prop->getIntMin();
	*upper = prop->getIntMax();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatPropertyRange(LavObject* obj, int slot, float* lower, float* upper) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT);
	*lower = prop->getFloatMin();
	*upper = prop->getFloatMax();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetDoublePropertyRange(LavObject* obj, int slot, double* lower, double* upper) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_DOUBLE);
	*lower = prop->getDoubleMin();
	*upper = prop->getDoubleMax();
	PUB_END
}

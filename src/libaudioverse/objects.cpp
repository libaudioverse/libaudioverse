/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all objects: linking, allocating, and freeing, as well as parent-child relationships.*/

#include <stdlib.h>
#include <string.h>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_macros.hpp>

float zerobuffer[Lav_MAX_BLOCK_SIZE] = {0}; //this is a shared buffer for the "no parent" case.

void LavObject::computeInputBuffers() {
	//point our inputs either at a zeroed buffer or the output of our parent.
	for(unsigned int i = 0; i < num_inputs; i++) {
		if(input_descriptors[i].parent != NULL) {
			inputs[i] = input_descriptors[i].parent->outputs[input_descriptors[i].output];
		}
		else {
			inputs[i] = zerobuffer;
		}
	}
}

void LavObject::init(LavDevice* device, unsigned int numInputs, unsigned int numOutputs) {
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
	computeInputBuffers(); //at the moment, this is going to just make them all 0, but it takes effect once parents are added.
}

void LavObject::process() {
	is_processing = 1;
	processor();
	is_processing = 0;
}

/*Default Processing function.*/
void LavObject::processor() {
	for(unsigned int i = 0; i < num_outputs; i++) {
		memset(outputs[i], 0, device->getBlockSize()*sizeof(float));
	}
}

void LavObject::setParent(unsigned int input, LavObject* parent, unsigned int parentOutput) {
	input_descriptors[input].parent = parent;
	input_descriptors[input].output = parentOutput;
	computeInputBuffers();
}

LavObject* LavObject::getParentObject(unsigned int slot) {
	return input_descriptors[slot].parent;
}

unsigned int LavObject::getParentOutput(unsigned int slot) {
	return input_descriptors[slot].output;
}

void LavObject::clearParent(unsigned int slot) {
	input_descriptors[slot].parent = nullptr;
	input_descriptors[slot].output = 0;
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

Lav_PUBLIC_FUNCTION LavError Lav_clearParent(LavObject *obj, unsigned int slot) {
	obj->clearParent(slot);
	return Lav_ERROR_NONE;
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

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_objectGetParent(LavObject *obj, unsigned int slot, LavObject** parent, unsigned int *outputNumber) {
	LOCK(*(obj->getDevice()));
	*parent = obj->getParentObject(slot);
	*outputNumber = obj->getParentOutput(slot);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetParent(LavObject *obj, unsigned int input, LavObject* parent, unsigned int output) {
	LOCK(*(obj->getDevice()));
	obj->setParent(input, parent, output);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectClearParent(LavObject *obj, unsigned int slot) {
	LOCK(*(obj->getDevice()));
	obj->clearParent(slot);
	return Lav_ERROR_NONE;
}

//this is properties.
//this is here because properties do not "know" about objects and only objects have properties; also, it made properties.cpp ahve to "know" about devices and objects.

//this works for getters and setters to lock the object and set a variable prop to be a pointer-like thing to a property.
#define PROP_PREAMBLE(o, s, t) LOCK(*(o));\
auto prop = (o)->getProperty((s));\
if(prop == nullptr) {\
return Lav_ERROR_RANGE;\
}\
if(prop->getType() != (t)) {\
return Lav_ERROR_TYPE_MISMATCH;\
}


Lav_PUBLIC_FUNCTION LavError Lav_objectResetProperty(LavObject *obj, unsigned int slot) {
	LOCK(*obj);
	auto prop = obj->getProperty(slot);
	if(prop == nullptr) return Lav_ERROR_RANGE;
	prop->reset();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetIntProperty(LavObject* obj, unsigned int slot, int value) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT);
	prop->setIntValue(value);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloatProperty(LavObject *obj, unsigned int slot, float value) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT);
	prop->setFloatValue(value);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetDoubleProperty(LavObject *obj, unsigned int slot, double value) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_DOUBLE);
	prop->setDoubleValue(value);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetStringProperty(LavObject*obj, unsigned int slot, char* value) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_STRING);
	prop->setStringValue(value);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloat3Property(LavObject* obj, unsigned int slot, float v1, float v2, float v3) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT3);
	prop->setFloat3Value(v1, v2, v3);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloat6Property(LavObject* obj, unsigned int slot, float v1, float v2, float v3, float v4, float v5, float v6) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT6);
	prop->setFloat6Value(v1, v2, v3, v4, v5, v6);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntProperty(LavObject*obj, unsigned int slot, int *destination) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT);
	*destination = prop->getIntValue();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatProperty(LavObject* obj, unsigned int slot, float *destination) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT);
	*destination = prop->getFloatValue();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetDoubleProperty(LavObject*obj, unsigned int slot, double *destination) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_DOUBLE);
	*destination = prop->getDoubleValue();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetStringProperty(LavObject* obj, unsigned int slot, const char** destination) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_STRING);
	*destination = prop->getStringValue();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloat3Property(LavObject* obj, unsigned int slot, float* v1, float* v2, float* v3) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT3);
	auto val = prop->getFloat3Value();
	*v1 = val[0];
	*v2 = val[1];
	*v3 = val[2];
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloat6Property(LavObject* obj, unsigned int slot, float* v1, float* v2, float* v3, float* v4, float* v5, float* v6) {
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT6);
	auto val = prop->getFloat6Value();
	*v1 = val[0];
	*v2 = val[1];
	*v3 = val[2];
	*v4 = val[3];
	*v5 = val[4];
	*v6 = val[5];
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntPropertyRange(LavObject* obj, unsigned int slot, int* lower, int* upper) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatPropertyRange(LavObject* obj, unsigned int slot, float* lower, float* upper) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetDoublePropertyRange(LavObject* obj, unsigned int slot, double* lower, double* upper) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

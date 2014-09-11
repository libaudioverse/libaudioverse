/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all objects: linking, allocating, and freeing, as well as parent-child relationships.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_metadata.hpp>
#include <algorithm>
#include <memory>
#include <stdlib.h>
#include <string.h>

float zerobuffer[Lav_MAX_BLOCK_SIZE] = {0}; //this is a shared buffer for the "no parent" case.

/**The following function verifies that, given two objects, an edge between them will not cause a cycle.
The edge is directed from start to end.*/
bool doesEdgePreserveAcyclicity(LavObject* start, LavObject* end) {
	std::set<LavObject*> checked;
	//this is a lambda trick to prevent needing to pass a third parameter around.
	std::function<bool(LavObject*)> bfs;
	bfs = [&](LavObject* what) {
		if(what == nullptr) return false; //the simplest base case: we were called on null.
		if(what == start) return true; //Travelling from end, we managed to reach start.
		if(checked.count(what)) return false; //we already checked this object and the graph is guaranteed to be static.
		for(unsigned int i = 0; i < what->getParentCount(); i++) {
			LavObject* par = what->getParentObject(i).get();
			if(bfs(par)) return true;
		}
		return false;
	};
	return bfs(end);
}


void LavObject::computeInputBuffers() {
	//point our inputs either at a zeroed buffer or the output of our parent.
	for(unsigned int i = 0; i < input_descriptors.size(); i++) {
		auto parent = input_descriptors[i].parent.lock();
		auto output = input_descriptors[i].output;
		if(parent != nullptr && parent->getState() != Lav_OBJSTATE_PAUSED) {
			if(output >= parent->getOutputCount()) { //the parent node no longer has this output, probably due to resizing.
				setParent(i, nullptr, 0); //so we clear this parent.
			}
			inputs[i] = parent->outputs[output];
		}
		else {
			inputs[i] = zerobuffer;
		}
	}
}

LavObject::LavObject(int type, std::shared_ptr<LavSimulation> simulation, unsigned int numInputs, unsigned int numOutputs): type(type) {
	//allocations:
	input_descriptors.resize(numInputs, LavInputDescriptor(nullptr, 0));
	inputs.resize(numInputs, nullptr);
	outputs.resize(numOutputs);
	for(auto i = outputs.begin(); i != outputs.end(); i++) {
		*i = new float[simulation->getBlockSize()];
	}

	this->simulation= simulation;
	//request properties from the metadata module.
	properties = makePropertyTable(type);
	//and callbacks.
	callbacks = makeCallbackTable(type);

	//Loop through callbacks, associating them with our simulation.
	//map iterators dont' give references, only operator[].
	for(auto i: callbacks) {
		callbacks[i.first].associateSimulation(simulation);
		callbacks[i.first].associateObject(this);
	}

	computeInputBuffers(); //at the moment, this is going to just make them all 0, but it takes effect once parents are added.
}

LavObject::~LavObject() {
	for(auto i: outputs) {
		delete[] i;
	}
	simulation->invalidatePlan();
}

void LavObject::willProcess() {
	is_processing = true;
	computeInputBuffers();
	num_inputs = inputs.size();
	num_outputs = outputs.size();
	block_size = simulation->getBlockSize();
}

/*Default Processing function.*/
void LavObject::process() {
	for(unsigned int i = 0; i < outputs.size(); i++) {
		memset(outputs[i], 0, block_size*sizeof(float));
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

int LavObject::getState() {
	return getProperty(Lav_OBJECT_STATE).getIntValue();
}

void LavObject::setParent(unsigned int input, std::shared_ptr<LavObject> parent, unsigned int parentOutput) {
	if(input >= input_descriptors.size()) throw LavErrorException(Lav_ERROR_RANGE);
	if(parent != nullptr && parentOutput >= parent->getOutputCount()) throw LavErrorException(Lav_ERROR_RANGE);
	//check acyclicity.
	if(doesEdgePreserveAcyclicity(this, parent.get())) throw LavErrorException(Lav_ERROR_CAUSES_CYCLE);
	input_descriptors[input].parent = parent;
	input_descriptors[input].output = parentOutput;
	simulation->invalidatePlan();
}

std::shared_ptr<LavObject> LavObject::getParentObject(unsigned int input) {
	if(input >= input_descriptors.size()) throw LavErrorException(Lav_ERROR_RANGE);
	return input_descriptors[input].parent.lock();
}

unsigned int LavObject::getParentOutput(unsigned int input) {
	if(input >= input_descriptors.size()) throw LavErrorException(Lav_ERROR_RANGE);
	auto parent = input_descriptors[input].parent.lock();
	return parent != nullptr ? input_descriptors[input].output : 0;
}

unsigned int LavObject::getParentCount() {
	return input_descriptors.size();
}

void LavObject::setInput(unsigned int input, std::shared_ptr<LavObject> object, unsigned int output) {
	setParent(input, object, output);
	if(getProperty(Lav_OBJECT_AUTORESET).getIntValue()) reset();
}

std::shared_ptr<LavObject> LavObject::getInputObject(unsigned int input) {
	return getParentObject(input);
}

unsigned int LavObject::getInputOutput(unsigned int input) {
	return getParentOutput(input);
}

unsigned int LavObject::getInputCount() {
	return getParentCount();
}

unsigned int LavObject::getOutputCount() {
	return outputs.size();
}

void LavObject::getOutputPointers(float** dest) {
	std::copy(outputs.begin(), outputs.end(), dest);
}

std::shared_ptr<LavSimulation> LavObject::getSimulation() {
	return simulation;
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

LavCallback& LavObject::getCallback(int which) {
	if(callbacks.count(which) == 0) throw LavErrorException(Lav_ERROR_RANGE);
	return callbacks[which];
}

void LavObject::lock() {
	simulation->lock();
}

void LavObject::unlock() {
	simulation->unlock();
}

void LavObject::reset() {
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
			outputs[i] = new float[simulation->getBlockSize()];
		}
	}
	simulation->invalidatePlan();
}

//LavSubgraphObject

LavSubgraphObject::LavSubgraphObject(int type, std::shared_ptr<LavSimulation> simulation): LavObject(type, simulation, 0, 0) {
}

void LavSubgraphObject::configureSubgraph(std::shared_ptr<LavObject> input, std::shared_ptr<LavObject> output) {
	subgraph_input = input;
	subgraph_output = output;
}

void LavSubgraphObject::computeInputBuffers() {
}

void LavSubgraphObject::process() {
//empty because we can forward onto the output object.
}

void LavSubgraphObject::setParent(unsigned int par, std::shared_ptr<LavObject> obj, unsigned int output) {
	throw LavErrorException(Lav_ERROR_INTERNAL);
}

std::shared_ptr<LavObject> LavSubgraphObject::getParentObject(unsigned int par) {
	if(par >= getParentCount()) throw LavErrorException(Lav_ERROR_RANGE);
	return subgraph_output;
}

unsigned int LavSubgraphObject::getParentOutput(unsigned int par) {
	if(par >= getParentCount()) throw LavErrorException(Lav_ERROR_RANGE);
	return par; //it's a one-for-one correspondance.
}

unsigned int LavSubgraphObject::getParentCount() {
	return subgraph_output ? subgraph_output->getParentCount() : 0;
}

void LavSubgraphObject::setInput(unsigned int input, std::shared_ptr<LavObject> object, unsigned int output) {
	if(input >= getInputCount()) throw LavErrorException(Lav_ERROR_RANGE);
	subgraph_input->setInput(input, object, output);
}

std::shared_ptr<LavObject> LavSubgraphObject::getInputObject(unsigned int input) {
	if(input >= getInputCount()) throw LavErrorException(Lav_ERROR_RANGE);
	return subgraph_input->getInputObject(input);
}

unsigned int LavSubgraphObject::getInputOutput(unsigned int input) {
	if(input >= getInputCount()) throw LavErrorException(Lav_ERROR_RANGE);
	return subgraph_input->getInputOutput(input);
}

unsigned int LavSubgraphObject::getInputCount() {
	if(subgraph_input == nullptr) return 0;
	return subgraph_input->getInputCount();
}

unsigned int LavSubgraphObject::getOutputCount() {
	if(subgraph_output) return subgraph_output->getOutputCount();
	else return 0;
}

void LavSubgraphObject::getOutputPointers(float** dest) {
	if(subgraph_output) subgraph_output->getOutputPointers(dest);
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_objectGetType(LavObject* obj, int* destination) {
	PUB_BEGIN
	auto obj_ptr = incomingPointer<LavObject>(obj);
	LOCK(*obj);
	*destination = obj->getType();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetInputCount(LavObject* obj, unsigned int* destination) {
	PUB_BEGIN
	auto obj_ptr = incomingPointer<LavObject>(obj);
	LOCK(*obj);
	*destination = obj->getInputCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetOutputCount(LavObject* obj, unsigned int* destination) {
	PUB_BEGIN
	auto obj_ptr = incomingPointer<LavObject>(obj);
	LOCK(*obj);
	*destination = obj->getOutputCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetInputObject(LavObject *obj, unsigned int slot, LavObject** destination) {
	PUB_BEGIN
	auto obj_ptr = incomingPointer<LavObject>(obj);
	LOCK(*obj);
	*destination = outgoingPointer<LavObject>(obj->getInputObject(slot));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetInputOutput(LavObject* obj, unsigned int slot, unsigned int* destination) {
	PUB_BEGIN
	auto obj_ptr = incomingPointer<LavObject>(obj);
	LOCK(*obj);
	*destination = obj->getInputOutput(slot);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetInput(LavObject *obj, unsigned int input, LavObject* parent, unsigned int output) {
	PUB_BEGIN
	auto obj_ptr = incomingPointer<LavObject>(obj);
	LOCK(*obj);
	obj->setInput(input, parent ? incomingPointer<LavObject>(parent) : nullptr, output);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectReset(LavObject* obj) {
	PUB_BEGIN
	LOCK(*obj);
	obj->reset();
	PUB_END
}

//this is properties.
//this is here because properties do not "know" about objects and only objects have properties; also, it made properties.cpp have to "know" about simulations and objects.

//this works for getters and setters to lock the object and set a variable prop to be a pointer-like thing to a property.
#define PROP_PREAMBLE(o, s, t) auto obj_ptr = incomingPointer<LavObject>(obj);\
LOCK(*(o));\
auto &prop = (o)->getProperty((s));\
if(prop.getType() != (t)) {\
throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);\
}

Lav_PUBLIC_FUNCTION LavError Lav_objectResetProperty(LavObject *obj, int slot) {
	PUB_BEGIN
	auto obj_ptr = incomingPointer<LavObject>(obj);
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
	auto obj_ptr = incomingPointer<LavObject>(obj);
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
	auto obj_ptr = incomingPointer<LavObject>(obj);
	LOCK(*obj);
	auto prop = obj->getProperty(slot);
	const char* n = prop.getName();
	char* dest = new char[strlen(n)+1]; //+1 for extra NULL.
	strcpy(dest, n);
	*destination = dest;
	PUB_END
}

//array properties.

Lav_PUBLIC_FUNCTION LavError Lav_objectReplaceFloatArrayProperty(LavObject* obj, int slot, unsigned int length, float* values) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	prop.replaceFloatArray(length, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectReadFloatArrayProperty(LavObject* obj, int slot, unsigned int index, float* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	*destination = prop.readFloatArray(index);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError  Lav_objectWriteFloatArrayProperty(LavObject* obj, int slot, unsigned int start, unsigned int stop, float* values) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	prop.writeFloatArray(start, stop, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatArrayPropertyDefault(LavObject* obj, int slot, unsigned int* destinationLength, float** destinationArray) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	auto def = prop.getFloatArrayDefault();
	if(def.size() == 0) {
		*destinationLength = 0;
		*destinationArray = nullptr;
		return Lav_ERROR_NONE;
	}
	float* buff = new float[def.size()];
	std::copy(def.begin(), def.end(), buff);
	auto del = [](float* what){delete[] what;};
	auto outgoing_buff = std::shared_ptr<float>(buff, del);
	*destinationLength = def.size();
	*destinationArray = outgoingPointer<float>(outgoing_buff);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatArrayPropertyLength(LavObject* obj, int slot, unsigned int* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	*destination = prop.getFloatArrayLength();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectReplaceIntArrayProperty(LavObject* obj, int slot, unsigned int length, int* values) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	prop.replaceIntArray(length, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectReadIntArrayProperty(LavObject* obj, int slot, unsigned int index, int* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	*destination = prop.readIntArray(index);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError  Lav_objectWriteIntArrayProperty(LavObject* obj, int slot, unsigned int start, unsigned int stop, int* values) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	prop.writeIntArray(start, stop, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntArrayPropertyDefault(LavObject* obj, int slot, unsigned int* destinationLength, int** destinationArray) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	auto def = prop.getIntArrayDefault();
	if(def.size() == 0) {
		*destinationLength = 0;
		*destinationArray = nullptr;
		return Lav_ERROR_NONE;
	}
	int* buff = new int[def.size()];
	std::copy(def.begin(), def.end(), buff);
	auto del = [](int* what){delete[] what;};
	auto outgoing_buff = std::shared_ptr<int>(buff, del);
	*destinationLength = def.size();
	*destinationArray = outgoingPointer<int>(outgoing_buff);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntArrayPropertyLength(LavObject* obj, int slot, int* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(obj, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	*destination = prop.getIntArrayLength();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetArrayPropertyLengthRange(LavObject* obj, int slot, unsigned int* destinationMin, unsigned int* destinationMax) {
	PUB_BEGIN
	auto ptr = incomingPointer<LavObject>(obj);
	LOCK(*obj);
	auto &prop = obj->getProperty(slot);
	int type = prop.getType();
	if(type != Lav_PROPERTYTYPE_FLOAT_ARRAY || type != Lav_PROPERTYTYPE_INT_ARRAY) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	PUB_END
}

//callback setup/configure/retrieval.

Lav_PUBLIC_FUNCTION LavError Lav_objectGetCallbackHandler(LavObject* obj, int callback, LavEventCallback *destination) {
	PUB_BEGIN
	auto ptr = incomingPointer<LavObject>(obj);
	LOCK(*obj);
	*destination = obj->getCallback(callback).getHandler();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectGetCallbackUserDataPointer(LavObject* obj, int callback, void** destination) {
	PUB_BEGIN
	auto ptr = incomingPointer<LavObject>(obj);
	LOCK(*obj);
	*destination = obj->getCallback(callback).getUserData();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_objectSetCallback(LavObject* obj, int callback, LavEventCallback handler, void* userData) {
	PUB_BEGIN
	auto ptr = incomingPointer<LavObject>(obj);
	LOCK(*ptr);
	obj->getCallback(callback).setHandler(handler);
	obj->getCallback(callback).setUserData(userData);
	PUB_END
}

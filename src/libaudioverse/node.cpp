/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all objects: linking, allocating, and freeing, as well as parent-child relationships.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/metadata.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <algorithm>
#include <memory>
#include <stdlib.h>
#include <string.h>

float zerobuffer[Lav_MAX_BLOCK_SIZE] = {0}; //this is a shared buffer for the "no parent" case.

/**The following function verifies that, given two objects, an edge between them will not cause a cycle.
The edge is directed from start to end.*/
bool doesEdgePreserveAcyclicity(LavNode* start, LavNode* end) {
	std::set<LavNode*> checked;
	//this is a lambda trick to prevent needing to pass a third parameter around.
	std::function<bool(LavNode*)> bfs;
	bfs = [&](LavNode* what) {
		if(what == nullptr) return false; //the simplest base case: we were called on null.
		if(what == start) return true; //Travelling from end, we managed to reach start.
		if(checked.count(what)) return false; //we already checked this object and the graph is guaranteed to be static.
		for(unsigned int i = 0; i < what->getParentCount(); i++) {
			LavNode* par = what->getParentNode(i).get();
			if(bfs(par)) return true;
		}
		return false;
	};
	return bfs(end);
}


void LavNode::computeInputBuffers() {
	//point our inputs either at a zeroed buffer or the output of our parent.
	for(unsigned int i = 0; i < input_descriptors.size(); i++) {
		auto parent = input_descriptors[i].parent.lock();
		auto output = input_descriptors[i].output;
		if(parent != nullptr && parent->getState() != Lav_NODESTATE_PAUSED) {
			if(output >= parent->getOutputCount()) { //the parent node no longer has this output, probably due to resizing.
				setParent(i, nullptr, 0); //so we clear this parent.
			}
			inputs[i] = parent->getOutputPointer(output);
		}
		else {
			inputs[i] = zerobuffer;
		}
	}
}

LavNode::LavNode(int type, std::shared_ptr<LavSimulation> simulation, unsigned int numInputs, unsigned int numOutputs): type(type) {
	//allocations:
	input_descriptors.resize(numInputs, LavInputDescriptor(nullptr, 0));
	inputs.resize(numInputs, nullptr);
	outputs.resize(numOutputs);
	for(auto i = outputs.begin(); i != outputs.end(); i++) {
		*i = LavAllocFloatArray(sizeof(float)*simulation->getBlockSize());
	}

	this->simulation= simulation;
	//request properties from the metadata module.
	properties = makePropertyTable(type);
	//and events.
	events = makeEventTable(type);

	//Loop through callbacks, associating them with our simulation.
	//map iterators dont' give references, only operator[].
	for(auto i: events) {
		events[i.first].associateSimulation(simulation);
		events[i.first].associateNode(this);
	}

	computeInputBuffers(); //at the moment, this is going to just make them all 0, but it takes effect once parents are added.

	//state changes must invalidate objects.
	getProperty(Lav_NODE_STATE).setPostChangedCallback([this]() {
		if(getProperty(Lav_NODE_STATE).getIntValue() == prev_state) return;
		this->simulation->invalidatePlan();
		prev_state = getProperty(Lav_NODE_STATE).getIntValue();
	});
}

LavNode::~LavNode() {
	for(auto i: outputs) {
		LavFreeFloatArray(i);
	}
	simulation->invalidatePlan();
}

void LavNode::tick() {
	if(last_processed== simulation->getTickCount()) return; //we processed this tick already.
	is_processing = true;
	computeInputBuffers();
	num_inputs = inputs.size();
	num_outputs = outputs.size();
	block_size = simulation->getBlockSize();
	process();
	float mul = getProperty(Lav_NODE_MUL).getFloatValue();
	if(mul != 1.0f) {
		for(unsigned int i = 0; i < getOutputCount(); i++) {
			float* output = getOutputPointer(i);
			scalarMultiplicationKernel(block_size, mul, output, output);
		}
	}
	is_processing = false;
	last_processed = simulation->getTickCount();
}

/*Default Processing function.*/
void LavNode::process() {
	for(unsigned int i = 0; i < outputs.size(); i++) {
		memset(outputs[i], 0, block_size*sizeof(float));
	}
}

void LavNode::willProcessParents() {
}

int LavNode::getType() {
	return type;
}

int LavNode::getState() {
	return getProperty(Lav_NODE_STATE).getIntValue();
}

void LavNode::setParent(unsigned int input, std::shared_ptr<LavNode> parent, unsigned int parentOutput) {
	if(input >= input_descriptors.size()) throw LavErrorException(Lav_ERROR_RANGE);
	if(parent != nullptr && parentOutput >= parent->getOutputCount()) throw LavErrorException(Lav_ERROR_RANGE);
	//check acyclicity.
	if(doesEdgePreserveAcyclicity(this, parent.get())) throw LavErrorException(Lav_ERROR_CAUSES_CYCLE);
	input_descriptors[input].parent = parent;
	input_descriptors[input].output = parentOutput;
	simulation->invalidatePlan();
}

std::shared_ptr<LavNode> LavNode::getParentNode(unsigned int input) {
	if(input >= input_descriptors.size()) throw LavErrorException(Lav_ERROR_RANGE);
	return input_descriptors[input].parent.lock();
}

unsigned int LavNode::getParentOutput(unsigned int input) {
	if(input >= input_descriptors.size()) throw LavErrorException(Lav_ERROR_RANGE);
	auto parent = input_descriptors[input].parent.lock();
	return parent != nullptr ? input_descriptors[input].output : 0;
}

unsigned int LavNode::getParentCount() {
	return input_descriptors.size();
}

void LavNode::setInput(unsigned int input, std::shared_ptr<LavNode> node, unsigned int output) {
	setParent(input, node, output);
	if(getProperty(Lav_NODE_AUTORESET).getIntValue()) reset();
}

std::shared_ptr<LavNode> LavNode::getInputNode(unsigned int input) {
	return getParentNode(input);
}

unsigned int LavNode::getInputOutput(unsigned int input) {
	return getParentOutput(input);
}

unsigned int LavNode::getInputCount() {
	return getParentCount();
}

unsigned int LavNode::getOutputCount() {
	return outputs.size();
}

float* LavNode::getOutputPointer(unsigned int output) {
	if(output > outputs.size()) throw LavErrorException(Lav_ERROR_RANGE);
	return outputs[output];
}

void LavNode::getOutputPointers(float** dest) {
	for(unsigned int i = 0; i < getOutputCount(); i++) dest[i] = getOutputPointer(i);
}

std::shared_ptr<LavSimulation> LavNode::getSimulation() {
	return simulation;
}

LavProperty& LavNode::getProperty(int slot) {
	if(properties.count(slot) == 0) throw LavErrorException(Lav_ERROR_RANGE);
	else return properties[slot];
}

std::vector<int> LavNode::getPropertyIndices() {
	std::vector<int> res;
	for(auto i = properties.begin(); i != properties.end(); i++) {
		res.push_back(i->first);
	}
	return res;
}

int LavNode::getPropertyCount() {
	return properties.size();
}

LavEvent& LavNode::getEvent(int which) {
	if(events.count(which) == 0) throw LavErrorException(Lav_ERROR_RANGE);
	return events[which];
}

void LavNode::lock() {
	simulation->lock();
}

void LavNode::unlock() {
	simulation->unlock();
}

void LavNode::reset() {
}

//protected resize function.
void LavNode::resize(unsigned int newInputCount, unsigned int newOutputCount) {
	//inputs is easy, because we didn't allocate any memory outside of the vector interface.
	inputs.resize(newInputCount, nullptr);
	input_descriptors.resize(newInputCount, LavInputDescriptor(nullptr, 0));
	//but outputs has a special case.
	unsigned int oldOutputCount = outputs.size();
	if(newOutputCount < oldOutputCount) { //we need to free some arrays.
		for(auto i = newOutputCount; i < oldOutputCount; i++) {
			LavFreeFloatArray(outputs[i]);
		}
	}
	//do the resize.
	outputs.resize(newOutputCount, nullptr);
	if(newOutputCount > oldOutputCount) { //we need to allocate some more arrays.
		for(auto i = oldOutputCount; i < newOutputCount; i++) {
			outputs[i] = LavAllocFloatArray(simulation->getBlockSize());
		}
	}
	simulation->invalidatePlan();
}

//LavSubgraphNode

LavSubgraphNode::LavSubgraphNode(int type, std::shared_ptr<LavSimulation> simulation): LavNode(type, simulation, 0, 0) {
}

void LavSubgraphNode::configureSubgraph(std::shared_ptr<LavNode> input, std::shared_ptr<LavNode> output) {
	subgraph_input = input;
	subgraph_output = output;
	simulation->invalidatePlan();
}

void LavSubgraphNode::computeInputBuffers() {
}

void LavSubgraphNode::doProcessProtocol() {
	//empty. Don't do anything for a subgraph.
}

void LavSubgraphNode::process() {
	//empty because we can forward onto the output object.
}

void LavSubgraphNode::setParent(unsigned int par, std::shared_ptr<LavNode> node, unsigned int output) {
	throw LavErrorException(Lav_ERROR_INTERNAL);
}

std::shared_ptr<LavNode> LavSubgraphNode::getParentNode(unsigned int par) {
	if(par >= getParentCount()) throw LavErrorException(Lav_ERROR_RANGE);
	return subgraph_output;
}

unsigned int LavSubgraphNode::getParentOutput(unsigned int par) {
	if(par >= getParentCount()) throw LavErrorException(Lav_ERROR_RANGE);
	return par; //it's a one-for-one correspondance.
}

unsigned int LavSubgraphNode::getParentCount() {
	return subgraph_output ? subgraph_output->getParentCount() : 0;
}

void LavSubgraphNode::setInput(unsigned int input, std::shared_ptr<LavNode> node, unsigned int output) {
	if(input >= getInputCount()) throw LavErrorException(Lav_ERROR_RANGE);
	subgraph_input->setInput(input, node, output);
}

std::shared_ptr<LavNode> LavSubgraphNode::getInputNode(unsigned int input) {
	if(input >= getInputCount()) throw LavErrorException(Lav_ERROR_RANGE);
	return subgraph_input->getInputNode(input);
}

unsigned int LavSubgraphNode::getInputOutput(unsigned int input) {
	if(input >= getInputCount()) throw LavErrorException(Lav_ERROR_RANGE);
	return subgraph_input->getInputOutput(input);
}

unsigned int LavSubgraphNode::getInputCount() {
	if(subgraph_input == nullptr) return 0;
	return subgraph_input->getInputCount();
}

unsigned int LavSubgraphNode::getOutputCount() {
	if(subgraph_output) return subgraph_output->getOutputCount();
	else return 0;
}

float* LavSubgraphNode::getOutputPointer(unsigned int output) {
	if(subgraph_output) return subgraph_output->getOutputPointer(output);
	else return nullptr;
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetType(LavNode* node, int* destination) {
	PUB_BEGIN
	auto node_ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	*destination = node->getType();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetInputCount(LavNode* node, unsigned int* destination) {
	PUB_BEGIN
	auto node_ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	*destination = node->getInputCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetOutputCount(LavNode* node, unsigned int* destination) {
	PUB_BEGIN
	auto node_ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	*destination = node->getOutputCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetInputNode(LavNode *node, unsigned int slot, LavNode** destination) {
	PUB_BEGIN
	auto node_ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	*destination = outgoingPointer<LavNode>(node->getInputNode(slot));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetInputOutput(LavNode* node, unsigned int slot, unsigned int* destination) {
	PUB_BEGIN
	auto node_ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	*destination = node->getInputOutput(slot);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetInput(LavNode *node, unsigned int input, LavNode* parent, unsigned int output) {
	PUB_BEGIN
	auto node_ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	node->setInput(input, parent ? incomingPointer<LavNode>(parent) : nullptr, output);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeReset(LavNode* node) {
	PUB_BEGIN
	LOCK(*node);
	node->reset();
	PUB_END
}

//this is properties.
//this is here because properties do not "know" about objects and only objects have properties; also, it made properties.cpp have to "know" about simulations and objects.

//this works for getters and setters to lock the object and set a variable prop to be a pointer-like thing to a property.
#define PROP_PREAMBLE(n, s, t) auto node_ptr = incomingPointer<LavNode>(n);\
LOCK(*(n));\
auto &prop = (n)->getProperty((s));\
if(prop.getType() != (t)) {\
throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);\
}

#define READONLY_CHECK if(prop.isReadOnly()) throw LavErrorException(Lav_ERROR_PROPERTY_IS_READ_ONLY);

Lav_PUBLIC_FUNCTION LavError Lav_nodeResetProperty(LavNode *node, int slot) {
	PUB_BEGIN
	auto node_ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	auto prop = node->getProperty(slot);
	READONLY_CHECK
	prop.reset();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetIntProperty(LavNode* node, int slot, int value) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_INT);
	READONLY_CHECK
	prop.setIntValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloatProperty(LavNode *node, int slot, float value) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT);
	READONLY_CHECK
	prop.setFloatValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetDoubleProperty(LavNode *node, int slot, double value) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_DOUBLE);
	READONLY_CHECK
	prop.setDoubleValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetStringProperty(LavNode*node, int slot, char* value) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_STRING);
	READONLY_CHECK
	prop.setStringValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloat3Property(LavNode* node, int slot, float v1, float v2, float v3) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT3);
	READONLY_CHECK
	prop.setFloat3Value(v1, v2, v3);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloat6Property(LavNode* node, int slot, float v1, float v2, float v3, float v4, float v5, float v6) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT6);
	READONLY_CHECK
	prop.setFloat6Value(v1, v2, v3, v4, v5, v6);
	return Lav_ERROR_NONE;
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntProperty(LavNode*node, int slot, int *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_INT);
	*destination = prop.getIntValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatProperty(LavNode* node, int slot, float *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT);
	*destination = prop.getFloatValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetDoubleProperty(LavNode*node, int slot, double *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_DOUBLE);
	*destination = prop.getDoubleValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetStringProperty(LavNode* node, int slot, const char** destination) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_STRING);
	*destination = prop.getStringValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloat3Property(LavNode* node, int slot, float* v1, float* v2, float* v3) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT3);
	auto val = prop.getFloat3Value();
	*v1 = val[0];
	*v2 = val[1];
	*v3 = val[2];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloat6Property(LavNode* node, int slot, float* v1, float* v2, float* v3, float* v4, float* v5, float* v6) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT6);
	auto val = prop.getFloat6Value();
	*v1 = val[0];
	*v2 = val[1];
	*v3 = val[2];
	*v4 = val[3];
	*v5 = val[4];
	*v6 = val[5];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntPropertyRange(LavNode* node, int slot, int* destination_lower, int* destination_upper) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_INT);
	*destination_lower = prop.getIntMin();
	*destination_upper = prop.getIntMax();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatPropertyRange(LavNode* node, int slot, float* destination_lower, float* destination_upper) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT);
	*destination_lower = prop.getFloatMin();
	*destination_upper = prop.getFloatMax();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetDoublePropertyRange(LavNode* node, int slot, double* destination_lower, double* destination_upper) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_DOUBLE);
	*destination_lower = prop.getDoubleMin();
	*destination_upper = prop.getDoubleMax();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyCount(LavNode* node, int* destination) {
	PUB_BEGIN
	LOCK(*node);
	*destination = node->getPropertyCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyIndices(LavNode* node, int** destination) {
	PUB_BEGIN
	auto node_ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	std::vector<int> indices = node->getPropertyIndices();
	int* result = new int[indices.size()+1]; //so we can terminate with 0.
	std::copy(indices.begin(), indices.end(), result);
	result[indices.size()] = 0;
	*destination = outgoingPointer<int>(std::shared_ptr<int>(result,
	[](int* ptr) {delete[] ptr;}));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyType(LavNode* node, int slot, int* destination) {
	PUB_BEGIN
	LOCK(*node);
	auto &prop = node->getProperty(slot);
	*destination = prop.getType();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyName(LavNode* node, int slot, char** destination) {
	PUB_BEGIN
	auto node_ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	auto prop = node->getProperty(slot);
	const char* n = prop.getName();
	char* dest = new char[strlen(n)+1]; //+1 for extra NULL.
	strcpy(dest, n);
	*destination = outgoingPointer<char>(std::shared_ptr<char>(dest,
	[](char* ptr) {delete[] ptr;}));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyHasDynamicRange(LavNode* node, int slot, int* destination) {
	PUB_BEGIN
	LOCK(*node);
	auto &prop = node->getProperty(slot);
	*destination = prop.getHasDynamicRange();
	PUB_END
}

//array properties.

Lav_PUBLIC_FUNCTION LavError Lav_nodeReplaceFloatArrayProperty(LavNode* node, int slot, unsigned int length, float* values) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	READONLY_CHECK
	prop.replaceFloatArray(length, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeReadFloatArrayProperty(LavNode* node, int slot, unsigned int index, float* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	*destination = prop.readFloatArray(index);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError  Lav_nodeWriteFloatArrayProperty(LavNode* node, int slot, unsigned int start, unsigned int stop, float* values) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	READONLY_CHECK
	prop.writeFloatArray(start, stop, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatArrayPropertyDefault(LavNode* node, int slot, unsigned int* destinationLength, float** destinationArray) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
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

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatArrayPropertyLength(LavNode* node, int slot, unsigned int* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	*destination = prop.getFloatArrayLength();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeReplaceIntArrayProperty(LavNode* node, int slot, unsigned int length, int* values) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	READONLY_CHECK
	prop.replaceIntArray(length, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeReadIntArrayProperty(LavNode* node, int slot, unsigned int index, int* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	*destination = prop.readIntArray(index);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError  Lav_nodeWriteIntArrayProperty(LavNode* node, int slot, unsigned int start, unsigned int stop, int* values) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	READONLY_CHECK
	prop.writeIntArray(start, stop, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntArrayPropertyDefault(LavNode* node, int slot, unsigned int* destinationLength, int** destinationArray) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_INT_ARRAY);
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

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntArrayPropertyLength(LavNode* node, int slot, int* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(node, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	*destination = prop.getIntArrayLength();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetArrayPropertyLengthRange(LavNode* node, int slot, unsigned int* destinationMin, unsigned int* destinationMax) {
	PUB_BEGIN
	auto ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	auto &prop = node->getProperty(slot);
	int type = prop.getType();
	if(type != Lav_PROPERTYTYPE_FLOAT_ARRAY || type != Lav_PROPERTYTYPE_INT_ARRAY) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	PUB_END
}

//callback setup/configure/retrieval.

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetEventHandler(LavNode* node, int event, LavEventCallback *destination) {
	PUB_BEGIN
	auto ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	*destination = node->getEvent(event).getExternalHandler();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetEventUserDataPointer(LavNode* node, int event, void** destination) {
	PUB_BEGIN
	auto ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	*destination = node->getEvent(event).getUserData();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetEvent(LavNode* node, int event, LavEventCallback handler, void* userData) {
	PUB_BEGIN
	auto ptr = incomingPointer<LavNode>(node);
	LOCK(*ptr);
	auto &ev = node->getEvent(event);
	if(handler) {
		ev.setHandler([=](LavNode* o, void* d) { handler(o, d);});
		ev.setExternalHandler(handler);
		ev.setUserData(userData);
	} else {
		ev.setHandler(std::function<void(LavNode*, void*)>());
		ev.setExternalHandler(nullptr);
	}
	PUB_END
}

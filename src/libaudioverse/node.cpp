/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all objects: linking, allocating, and freeing, as well as parent-child relationships.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/connections.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/metadata.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <algorithm>
#include <memory>
#include <stdlib.h>
#include <string.h>

/**The following function verifies that, given two objects, an edge between them will not cause a cycle.
The edge is directed from start to end.*/
bool doesEdgePreserveAcyclicity(LavNode* start, LavNode* end) {
	return true; //lie, for now.
}

LavNode::LavNode(int type, std::shared_ptr<LavSimulation> simulation, unsigned int numInputBuffers, unsigned int numOutputBuffers): type(type) {
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

	//allocations can be done simply by redirecting through resize after our initialization step.
	resize(numInputBuffers, numOutputBuffers);
}

LavNode::~LavNode() {
	for(auto i: output_buffers) {
		LavFreeFloatArray(i);
	}
	for(auto i: input_buffers) {
		LavFreeFloatArray(i);
	}
}

void LavNode::tick() {
	if(last_processed== simulation->getTickCount()) return; //we processed this tick already.
	//Incrementing this counter here prevents duplication of zeroing outputs if we're in the paused state.
	last_processed = simulation->getTickCount();
	zeroOutputBuffers(); //we always do this because sometimes we're not going to actually do anything else.
	if(getState() == Lav_NODESTATE_PAUSED) return; //nothing to do, for we are paused.
	willProcessParents();
	zeroInputBuffers();
	//tick all alive parents, collecting their outputs onto ours.
	//by using the getInputConnection and getInputConnectionCount functions, we allow subgraphs to override effectively.
	for(int i = 0; i < getInputConnectionCount(); i++) {
		getInputConnection(i)->add(true); //for now, always apply the mixing matrix.
	}
	is_processing = true;
	num_input_buffers = input_buffers.size();
	num_output_buffers = output_buffers.size();
	block_size = simulation->getBlockSize();
	process();
	float mul = getProperty(Lav_NODE_MUL).getFloatValue();
	if(mul != 1.0f) {
		for(unsigned int i = 0; i < getOutputBufferCount(); i++) {
			float* output = getOutputBufferArray()[i];
			scalarMultiplicationKernel(block_size, mul, output, output);
		}
	}
	is_processing = false;
}

//cleans up stuff.
void LavNode::doMaintenance() {
	//nothing, for now. This is needed for the upcoming refactor.
}

/*Default Processing function.*/
void LavNode::process() {
	zeroOutputBuffers();
}

void LavNode::zeroOutputBuffers() {
	for(unsigned int i = 0; i < output_buffers.size(); i++) {
		memset(output_buffers[i], 0, block_size*sizeof(float));
	}
}

void LavNode::zeroInputBuffers() {
	for(int i = 0; i < input_buffers.size(); i++) {
		memset(input_buffers[i], 0, sizeof(float)*simulation->getBlockSize());
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

unsigned int LavNode::getOutputBufferCount() {
	return output_buffers.size();
}

float** LavNode::getOutputBufferArray() {
	//vectors are guaranteed to be contiguous in most if not all implementations as well as (possibly, no source handy) the C++11 standard.
	return &output_buffers[0];
}

int LavNode::getInputBufferCount() {
	return input_buffers.size();
}

float** LavNode::getInputBufferArray() {
	return &input_buffers[0];
}

int LavNode::getInputConnectionCount() {
	return input_connections.size();
}

int LavNode::getOutputConnectionCount() {
	return output_connections.size();
}

std::shared_ptr<LavInputConnection> LavNode::getInputConnection(int which) {
	if(which >= getInputConnectionCount() || which < 0) throw LavErrorException(Lav_ERROR_RANGE);
	return std::shared_ptr<LavInputConnection>(this->shared_from_this(), &input_connections[which]);
}

std::shared_ptr<LavOutputConnection> LavNode::getOutputConnection(int which) {
	if(which < 0 || which >= getOutputConnectionCount()) throw LavErrorException(Lav_ERROR_RANGE);
	return std::shared_ptr<LavOutputConnection>(this->shared_from_this(), &output_connections[which]);
}

void LavNode::appendInputConnection(int start, int count) {
	input_connections.emplace_back(simulation, this, start, count);
}

void LavNode::appendOutputConnection(int start, int count) {
	output_connections.emplace_back(simulation, this, start, count);
}

void LavNode::connect(int output, std::shared_ptr<LavNode> toNode, int input) {
	auto outputConnection =getOutputConnection(output);
	auto inputConnection = toNode->getInputConnection(input);
	makeConnection(outputConnection, inputConnection);
}

void LavNode::disconnect(int which) {
	auto o =getOutputConnection(which);
	o->clear();
}

std::shared_ptr<LavSimulation> LavNode::getSimulation() {
	return simulation;
}

LavProperty& LavNode::getProperty(int slot) {
	if(properties.count(slot) == 0) throw LavErrorException(Lav_ERROR_RANGE);
	else return properties[slot];
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
void LavNode::resize(int newInputCount, int newOutputCount) {
	int oldInputCount = input_buffers.size();
	for(int i = oldInputCount-1; i >= newInputCount; i--) LavFreeFloatArray(input_buffers[i]);
	input_buffers.resize(newInputCount, nullptr);
	for(int i = oldInputCount; i < newInputCount; i++) input_buffers[i]=LavAllocFloatArray(simulation->getBlockSize());

	int oldOutputCount = output_buffers.size();
	if(newOutputCount < oldOutputCount) { //we need to free some arrays.
		for(auto i = newOutputCount; i < oldOutputCount; i++) {
			LavFreeFloatArray(output_buffers[i]);
		}
	}
	//do the resize.
	output_buffers.resize(newOutputCount, nullptr);
	if(newOutputCount > oldOutputCount) { //we need to allocate some more arrays.
		for(auto i = oldOutputCount; i < newOutputCount; i++) {
			output_buffers[i] = LavAllocFloatArray(simulation->getBlockSize());
		}
	}
}

//LavSubgraphNode

LavSubgraphNode::LavSubgraphNode(int type, std::shared_ptr<LavSimulation> simulation): LavNode(type, simulation, 0, 0) {
}

void LavSubgraphNode::configureSubgraph(std::shared_ptr<LavNode> input, std::shared_ptr<LavNode> output) {
	subgraph_input = input;
	subgraph_output = output;
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_nodeConnect(LavNode* node, int output, LavNode* dest, int input) {
	PUB_BEGIN
	LOCK(*node);
	node->connect(output, incomingPointer<LavNode>(dest), input);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeDisconnect(LavNode* node, int output) {
	PUB_BEGIN
	LOCK(*node);
	node->disconnect(output);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetType(LavNode* node, int* destination) {
	PUB_BEGIN
	auto node_ptr = incomingPointer<LavNode>(node);
	LOCK(*node);
	*destination = node->getType();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetInputCount(LavNode* node, unsigned int* destination) {
	PUB_BEGIN
	//todo:rewrite to handle the "new" idea of inputs.
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetOutputCount(LavNode* node, unsigned int* destination) {
	PUB_BEGIN
	//todo:rewrite to handle the "new" idea of outputs.
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

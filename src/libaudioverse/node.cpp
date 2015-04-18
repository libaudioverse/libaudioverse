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
#include <libaudioverse/private/buffer.hpp>
#include <algorithm>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <vector>

/**Given two nodes, determine if connecting an output of start to an input of end causes a cycle.*/
bool doesEdgePreserveAcyclicity(std::shared_ptr<LavNode> start, std::shared_ptr<LavNode> end) {
	//A cycle exists if end is directly or indirectly conneccted to an input of start.
	//To that end, we use recursion as follows.
	//if we are called with start==end, it's a cycle.
	if(start==end) return false;
	//Otherwise, move end back a level.
	//the rationale for this is that we want to seeee if an indirect connection from anything we're pulling from to our end causes a cycle.  If it doesn't, we're good.
	for(auto n: end->getDependencies()) {
		if(doesEdgePreserveAcyclicity(start, n) == false) return false;
	}
	return true;
}

LavNode::LavNode(int type, std::shared_ptr<LavSimulation> simulation, unsigned int numInputBuffers, unsigned int numOutputBuffers): LavExternalObject(type) {
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
	bool needsMixing = getProperty(Lav_NODE_CHANNEL_INTERPRETATION).getIntValue()==Lav_CHANNEL_INTERPRETATION_SPEAKERS;
	for(int i = 0; i < getInputConnectionCount(); i++) {
		getInputConnection(i)->add(needsMixing);
	}
	is_processing = true;
	num_input_buffers = input_buffers.size();
	num_output_buffers = output_buffers.size();
	block_size = simulation->getBlockSize();
	process();
	for(int i = 0; i < getOutputBufferCount(); i++) {
		float* output = getOutputBufferArray()[i];
		for(int j = 0; j < block_size; j++) {
			float mul = getProperty(Lav_NODE_MUL).getFloatValue();
			output[j]*=mul;
		}
	}
	for(int i = 0; i < getOutputBufferCount(); i++) {
		float* output = getOutputBufferArray()[i];
		for(int j = 0; j < block_size; j++) {
			float add=getProperty(Lav_NODE_ADD).getFloatValue(j);
			output[j] += add;
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
	for(unsigned int i = 0; i < input_buffers.size(); i++) {
		memset(input_buffers[i], 0, sizeof(float)*simulation->getBlockSize());
	}
}

void LavNode::willProcessParents() {
}

int LavNode::getState() {
	return getProperty(Lav_NODE_STATE).getIntValue();
}

int LavNode::getOutputBufferCount() {
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
	if(doesEdgePreserveAcyclicity(std::static_pointer_cast<LavNode>(this->shared_from_this()), toNode) == false) throw LavErrorException(Lav_ERROR_CAUSES_CYCLE);
	auto outputConnection =getOutputConnection(output);
	auto inputConnection = toNode->getInputConnection(input);
	makeConnection(outputConnection, inputConnection);
}

void LavNode::connectSimulation(int which) {
	auto outputConnection=getOutputConnection(which);
	auto inputConnection = simulation->getFinalOutputConnection();
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

std::set<std::shared_ptr<LavNode>> LavNode::getDependencies() {
	std::set<std::shared_ptr<LavNode>> retval;
	for(int i = 0; i < getInputConnectionCount(); i++) {
		auto j = getInputConnection(i)->getConnectedNodes();
		for(auto p: j) {
			retval.insert(std::static_pointer_cast<LavNode>(p->shared_from_this()));
		}
	}
	return retval;
}

//LavSubgraphNode

LavSubgraphNode::LavSubgraphNode(int type, std::shared_ptr<LavSimulation> simulation): LavNode(type, simulation, 0, 0) {
}

void LavSubgraphNode::setInputNode(std::shared_ptr<LavNode> node) {
	subgraph_input= node;
}

void LavSubgraphNode::setOutputNode(std::shared_ptr<LavNode> node) {
	subgraph_output=node;
}

int LavSubgraphNode::getInputConnectionCount() {
	if(subgraph_input) return subgraph_input->getInputConnectionCount();
	else return 0;
}

std::shared_ptr<LavInputConnection> LavSubgraphNode::getInputConnection(int which) {
	if(which < 0|| which >= getInputConnectionCount()) throw LavErrorException(Lav_ERROR_RANGE);
	else return subgraph_input->getInputConnection(which);
}

int LavSubgraphNode::getOutputBufferCount() {
	if(subgraph_output) return subgraph_output->getOutputBufferCount();
	else return 0;
}

float** LavSubgraphNode::getOutputBufferArray() {
	if(subgraph_output) return subgraph_output->getOutputBufferArray();
	return nullptr;
}

void LavSubgraphNode::tick() {
	if(last_processed== simulation->getTickCount()) return;
	last_processed=simulation->getTickCount();
	if(getState() == Lav_NODESTATE_PAUSED) return;
	willProcessParents();
	if(subgraph_output == nullptr) return;
	subgraph_output->tick();
	//Handle our add and mul, on top of the output object of the subgraph.
	//We prefer this over forwarding because this allows the subgraph to change all internal volumes without them being overridden by the user.
	float mul = getProperty(Lav_NODE_MUL).getFloatValue();
	if(mul != 1.0f) {
		for(int i = 0; i < subgraph_output->getOutputBufferCount(); i++) {
			float* output = subgraph_output->getOutputBufferArray()[i];
			scalarMultiplicationKernel(simulation->getBlockSize(), mul, output, output);
		}
	}
	float add=getProperty(Lav_NODE_ADD).getFloatValue();
	if(add != 0.0f) {
		for(int i = 0; i < subgraph_output->getOutputBufferCount(); i++) {
			scalarAdditionKernel(simulation->getBlockSize(), add, subgraph_output->getOutputBufferArray()[i], getOutputBufferArray()[i]);
		}
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetSimulation(LavHandle handle, LavHandle* destination) {
	PUB_BEGIN
auto n = incomingObject<LavNode>(handle);
	*destination = outgoingObject(n->getSimulation());
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeConnect(LavHandle nodeHandle, int output, LavHandle destHandle, int input) {
	PUB_BEGIN
	auto node= incomingObject<LavNode>(nodeHandle);
	auto dest = incomingObject<LavNode>(destHandle);
	LOCK(*node);
	node->connect(output, dest, input);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeConnectSimulation(LavHandle nodeHandle, int output) {
	PUB_BEGIN
	auto node = incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	node->connectSimulation(output);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeDisconnect(LavHandle nodeHandle, int output) {
	PUB_BEGIN
	auto node = incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	node->disconnect(output);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetInputConnectionCount(LavHandle nodeHandle, unsigned int* destination) {
	PUB_BEGIN
	auto node = incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	*destination =node->getInputConnectionCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetOutputConnectionCount(LavHandle nodeHandle, unsigned int* destination) {
	PUB_BEGIN
	auto node = incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	*destination = node->getOutputConnectionCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeReset(LavHandle nodeHandle) {
	PUB_BEGIN
	auto node = incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	node->reset();
	PUB_END
}

//this is properties.
//this is here because properties do not "know" about objects and only objects have properties; also, it made properties.cpp have to "know" about simulations and objects.

//this works for getters and setters to lock the object and set a variable prop to be a pointer-like thing to a property.
#define PROP_PREAMBLE(n, s, t) auto node_ptr = incomingObject<LavNode>(n);\
LOCK(*node_ptr);\
auto &prop = node_ptr->getProperty((s));\
if(prop.getType() != (t)) {\
throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);\
}

#define READONLY_CHECK if(prop.isReadOnly()) throw LavErrorException(Lav_ERROR_PROPERTY_IS_READ_ONLY);

Lav_PUBLIC_FUNCTION LavError Lav_nodeResetProperty(LavHandle nodeHandle, int slot) {
	PUB_BEGIN
	auto node_ptr = incomingObject<LavNode>(nodeHandle);
	LOCK(*node_ptr);
	auto prop = node_ptr->getProperty(slot);
	READONLY_CHECK
	prop.reset();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetIntProperty(LavHandle nodeHandle, int slot, int value) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_INT);
	READONLY_CHECK
	prop.setIntValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloatProperty(LavHandle nodeHandle, int slot, float value) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT);
	READONLY_CHECK
	prop.setFloatValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetDoubleProperty(LavHandle nodeHandle, int slot, double value) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_DOUBLE);
	READONLY_CHECK
	prop.setDoubleValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetStringProperty(LavHandle nodeHandle, int slot, char* value) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_STRING);
	READONLY_CHECK
	prop.setStringValue(value);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloat3Property(LavHandle nodeHandle, int slot, float v1, float v2, float v3) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT3);
	READONLY_CHECK
	prop.setFloat3Value(v1, v2, v3);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloat6Property(LavHandle nodeHandle, int slot, float v1, float v2, float v3, float v4, float v5, float v6) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT6);
	READONLY_CHECK
	prop.setFloat6Value(v1, v2, v3, v4, v5, v6);
	return Lav_ERROR_NONE;
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntProperty(LavHandle nodeHandle, int slot, int *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_INT);
	*destination = prop.getIntValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatProperty(LavHandle nodeHandle, int slot, float *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT);
	*destination = prop.getFloatValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetDoubleProperty(LavHandle nodeHandle, int slot, double *destination) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_DOUBLE);
	*destination = prop.getDoubleValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetStringProperty(LavHandle nodeHandle, int slot, const char** destination) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_STRING);
	*destination = prop.getStringValue();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloat3Property(LavHandle nodeHandle, int slot, float* v1, float* v2, float* v3) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT3);
	auto val = prop.getFloat3Value();
	*v1 = val[0];
	*v2 = val[1];
	*v3 = val[2];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloat6Property(LavHandle nodeHandle, int slot, float* v1, float* v2, float* v3, float* v4, float* v5, float* v6) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT6);
	auto val = prop.getFloat6Value();
	*v1 = val[0];
	*v2 = val[1];
	*v3 = val[2];
	*v4 = val[3];
	*v5 = val[4];
	*v6 = val[5];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntPropertyRange(LavHandle nodeHandle, int slot, int* destination_lower, int* destination_upper) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_INT);
	*destination_lower = prop.getIntMin();
	*destination_upper = prop.getIntMax();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatPropertyRange(LavHandle nodeHandle, int slot, float* destination_lower, float* destination_upper) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT);
	*destination_lower = prop.getFloatMin();
	*destination_upper = prop.getFloatMax();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetDoublePropertyRange(LavHandle nodeHandle, int slot, double* destination_lower, double* destination_upper) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_DOUBLE);
	*destination_lower = prop.getDoubleMin();
	*destination_upper = prop.getDoubleMax();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyType(LavHandle nodeHandle, int slot, int* destination) {
	PUB_BEGIN
	auto node= incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	auto &prop = node->getProperty(slot);
	*destination = prop.getType();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyName(LavHandle nodeHandle, int slot, char** destination) {
	PUB_BEGIN
	auto node_ptr = incomingObject<LavNode>(nodeHandle);
	LOCK(*node_ptr);
	auto prop = node_ptr->getProperty(slot);
	const char* n = prop.getName();
	char* dest = new char[strlen(n)+1]; //+1 for extra NULL.
	strcpy(dest, n);
	*destination = outgoingPointer<char>(std::shared_ptr<char>(dest,
	[](char* ptr) {delete[] ptr;}));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyHasDynamicRange(LavHandle nodeHandle, int slot, int* destination) {
	PUB_BEGIN
	auto node = incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	auto &prop = node->getProperty(slot);
	*destination = prop.getHasDynamicRange();
	PUB_END
}

//array properties.

Lav_PUBLIC_FUNCTION LavError Lav_nodeReplaceFloatArrayProperty(LavHandle nodeHandle, int slot, unsigned int length, float* values) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	READONLY_CHECK
	prop.replaceFloatArray(length, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeReadFloatArrayProperty(LavHandle nodeHandle, int slot, unsigned int index, float* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	*destination = prop.readFloatArray(index);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError  Lav_nodeWriteFloatArrayProperty(LavHandle nodeHandle, int slot, unsigned int start, unsigned int stop, float* values) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	READONLY_CHECK
	prop.writeFloatArray(start, stop, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatArrayPropertyDefault(LavHandle nodeHandle, int slot, unsigned int* destinationLength, float** destinationArray) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
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

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatArrayPropertyLength(LavHandle nodeHandle, int slot, unsigned int* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_FLOAT_ARRAY);
	*destination = prop.getFloatArrayLength();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeReplaceIntArrayProperty(LavHandle nodeHandle, int slot, unsigned int length, int* values) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	READONLY_CHECK
	prop.replaceIntArray(length, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeReadIntArrayProperty(LavHandle nodeHandle, int slot, unsigned int index, int* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	*destination = prop.readIntArray(index);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError  Lav_nodeWriteIntArrayProperty(LavHandle nodeHandle, int slot, unsigned int start, unsigned int stop, int* values) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	READONLY_CHECK
	prop.writeIntArray(start, stop, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntArrayPropertyDefault(LavHandle nodeHandle, int slot, unsigned int* destinationLength, int** destinationArray) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_INT_ARRAY);
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

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntArrayPropertyLength(LavHandle nodeHandle, int slot, int* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_INT_ARRAY);
	*destination = prop.getIntArrayLength();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetArrayPropertyLengthRange(LavHandle nodeHandle, int slot, unsigned int* destinationMin, unsigned int* destinationMax) {
	PUB_BEGIN
	auto ptr = incomingObject<LavNode>(nodeHandle);
	LOCK(*ptr);
	auto &prop = ptr->getProperty(slot);
	int type = prop.getType();
	if(type != Lav_PROPERTYTYPE_FLOAT_ARRAY || type != Lav_PROPERTYTYPE_INT_ARRAY) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetBufferProperty(LavHandle nodeHandle, int slot, LavHandle bufferHandle) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_BUFFER);
	auto buff=incomingObject<LavBuffer>(bufferHandle, true);
	prop.setBufferValue(buff);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetBufferProperty(LavHandle nodeHandle, int slot, LavHandle* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_BUFFER);
	*destination = outgoingObject(prop.getBufferValue());
	PUB_END
}

//callback setup/configure/retrieval.
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetEventHandler(LavHandle nodeHandle, int event, LavEventCallback *destination) {
	PUB_BEGIN
	auto ptr = incomingObject<LavNode>(nodeHandle);
	LOCK(*ptr);
	*destination = ptr->getEvent(event).getExternalHandler();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetEventUserDataPointer(LavHandle nodeHandle, int event, void** destination) {
	PUB_BEGIN
	auto ptr = incomingObject<LavNode>(nodeHandle);
	LOCK(*ptr);
	*destination = ptr->getEvent(event).getUserData();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetEvent(LavHandle nodeHandle, int event, LavEventCallback handler, void* userData) {
	PUB_BEGIN
	auto ptr = incomingObject<LavNode>(nodeHandle);
	LOCK(*ptr);
	auto &ev = ptr->getEvent(event);
	if(handler) {
		ev.setHandler([=](LavNode* o, void* d) { handler(o->externalObjectHandle, d);});
		ev.setExternalHandler(handler);
		ev.setUserData(userData);
	} else {
		ev.setHandler(std::function<void(LavNode*, void*)>());
		ev.setExternalHandler(nullptr);
	}
	PUB_END
}

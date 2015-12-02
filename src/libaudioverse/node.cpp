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
#include <libaudioverse/private/dependency_computation.hpp>
#include <algorithm>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <vector>

namespace libaudioverse_implementation {


/**Given two nodes, determine if connecting an output of start to an input of end causes a cycle.*/
bool doesEdgePreserveAcyclicity(std::shared_ptr<Job> start, std::shared_ptr<Job> end) {
	//A cycle exists if end is directly or indirectly conneccted to an input of start.
	//To that end, we use recursion as follows.
	//if we are called with start==end, it's a cycle.
	if(start==end) return false;
	//Inductive step:
	//connecting start to end connects everything "behind" start to end,
	//so there's a cycle if end is already behind start.
	//We check by walking all dependencies of start looking for end.
	bool cycled = false;
	auto helper = [&](std::shared_ptr<Job> current, auto callable) {
		cycled = current == end;
		if(cycled) return;
		//We're passing ourself to ourself to avoid std::functions all the way down.
		else visitDependencies(current, callable, callable);
	};
	//And then we pass it to itself.
	visitDependencies(start, helper, helper);
	return cycled == false;
}

//For property backrefs.
bool PropertyBackrefComparer::operator() (const std::tuple<std::weak_ptr<Node>, int> &a, const std::tuple<std::weak_ptr<Node>, int> &b) const {
	auto &aw = std::get<0>(a);
	auto &bw = std::get<0>(b);
	if(aw.owner_before(bw)) return true;
	else if(bw.owner_before(aw)) return false;
	else return std::get<1>(a) < std::get<1>(b);
}

Node::Node(int type, std::shared_ptr<Simulation> simulation, unsigned int numInputBuffers, unsigned int numOutputBuffers): Job(type) {
	this->simulation= simulation;
	//request properties from the metadata module.
	properties = makePropertyTable(type);
	//Associate properties to this node:
	for(auto i: properties) {
		auto &prop = properties[i.first];
		prop.associateNode(this);
		prop.associateSimulation(simulation);
	}

	//allocations can be done simply by redirecting through resize after our initialization step.
	resize(numInputBuffers, numOutputBuffers);
	
	//Block sizes never change:
	block_size = simulation->getBlockSize();
	
	//We must invalidate the plan when people touch the state property.
	getProperty(Lav_NODE_STATE).setPostChangedCallback([&] () {stateChanged();});
}

Node::~Node() {
	for(auto i: output_buffers) {
		if(i) freeArray(i);
	}
	for(auto i: input_buffers) {
		if(i) freeArray(i);
	}
	simulation->invalidatePlan();
}

void Node::tickProperties() {
	for(auto &i: properties) {
		i.second.tick();
	}
}

void Node::tick() {
	last_processed = simulation->getTickCount();
	if(getState() == Lav_NODESTATE_PAUSED) return; //nothing to do, for we are paused.
	//If we're paused, then our output connections short-circuit and add zero.
	zeroOutputBuffers();
	tickProperties();
	zeroInputBuffers();
	//Collect parent outputs onto ours.
	//by using the getInputConnection and getInputConnectionCount functions, we allow subgraphs to override effectively.
	bool needsMixing = getProperty(Lav_NODE_CHANNEL_INTERPRETATION).getIntValue()==Lav_CHANNEL_INTERPRETATION_SPEAKERS;
	for(int i = 0; i < getInputConnectionCount(); i++) {
		getInputConnection(i)->add(needsMixing);
	}
	is_processing = true;
	num_input_buffers = input_buffers.size();
	num_output_buffers = output_buffers.size();
	process();
	applyMul();
	applyAdd();
	is_processing = false;
}

void Node::applyMul() {
	auto &mulProp = getProperty(Lav_NODE_MUL);
	float** outputs =getOutputBufferArray();
	if(mulProp.needsARate()) {
		for(int i = 0; i < block_size; i++) {
			float mul = mulProp.getFloatValue(i);
			for(int j = 0; j < getOutputBufferCount(); j++) outputs[j][i]*=mul;
		}
	}
	else if(mulProp.getFloatValue() !=1.0) {
		for(int i = 0; i < getOutputBufferCount(); i++) {
			scalarMultiplicationKernel(block_size, mulProp.getFloatValue(), outputs[i], outputs[i]);
		}
	}
}

void Node::applyAdd() {
	auto &addProp = getProperty(Lav_NODE_ADD);
	float** outputs = getOutputBufferArray();
	if(addProp.needsARate()) {
		for(int i = 0; i < block_size; i++) {
		float add=addProp.getFloatValue(i);
			for(int j = 0; j < getOutputBufferCount(); j++) outputs[j][i]+=add;
		}
	}
	else if(addProp.getFloatValue() !=0.0) {
		for(int i = 0; i < getOutputBufferCount(); i++) {
			scalarAdditionKernel(block_size, addProp.getFloatValue(), outputs[i], outputs[i]);
		}
	}
}

//cleans up stuff.
void Node::doMaintenance() {
	//nothing, for now. This is needed for the upcoming refactor.
}

/*Default Processing function.*/
void Node::process() {
}

void Node::zeroOutputBuffers() {
	float** outputBuffers=getOutputBufferArray();
	int outputBufferCount = getOutputBufferCount();
	for(int i = 0; i < outputBufferCount; i++) {
		memset(outputBuffers[i], 0, block_size*sizeof(float));
	}
}

void Node::zeroInputBuffers() {
	int inputBufferCount=getInputBufferCount();
	float** inputBuffers=getInputBufferArray();
	for(int i = 0; i < inputBufferCount; i++) {
		memset(inputBuffers[i], 0, sizeof(float)*simulation->getBlockSize());
	}
}

void Node::willTick() {
}

int Node::getState() {
	return getProperty(Lav_NODE_STATE).getIntValue();
}

void Node::setState(int newState) {
	getProperty(Lav_NODE_STATE).setIntValue(newState);
}

void Node::stateChanged() {
	if(getState() == prev_state) return;
	simulation->invalidatePlan();
	if(prev_state == Lav_NODESTATE_ALWAYS_PLAYING) simulation->unregisterNodeForAlwaysPlaying(std::static_pointer_cast<Node>(shared_from_this()));
	prev_state = getProperty(Lav_NODE_STATE).getIntValue();
	if(prev_state == Lav_NODESTATE_ALWAYS_PLAYING) simulation->registerNodeForAlwaysPlaying(std::static_pointer_cast<Node>(shared_from_this()));
}

int Node::getOutputBufferCount() {
	return output_buffers.size();
}

float** Node::getOutputBufferArray() {
	//vectors are guaranteed to be contiguous in most if not all implementations as well as (possibly, no source handy) the C++11 standard.
	if(output_buffers.size())  return &output_buffers[0];
	else return nullptr;
}

int Node::getInputBufferCount() {
	return input_buffers.size();
}

float** Node::getInputBufferArray() {
	if(input_buffers.size()) return &input_buffers[0];
	else return nullptr;
}

int Node::getInputConnectionCount() {
	return input_connections.size();
}

int Node::getOutputConnectionCount() {
	return output_connections.size();
}

std::shared_ptr<InputConnection> Node::getInputConnection(int which) {
	if(which >= getInputConnectionCount() || which < 0) ERROR(Lav_ERROR_RANGE, "Invalid input.");
	return input_connections[which];
}

std::shared_ptr<OutputConnection> Node::getOutputConnection(int which) {
	if(which < 0 || which >= getOutputConnectionCount()) ERROR(Lav_ERROR_RANGE, "Invalid output.");
	return output_connections[which];
}

void Node::appendInputConnection(int start, int count) {
	input_connections.emplace_back(new InputConnection(simulation, this, start, count));
}

void Node::appendOutputConnection(int start, int count) {
	output_connections.emplace_back(new OutputConnection(simulation, this, start, count));
}

void Node::connect(int output, std::shared_ptr<Node> toNode, int input) {
	if(doesEdgePreserveAcyclicity(std::static_pointer_cast<Node>(this->shared_from_this()), toNode) == false) ERROR(Lav_ERROR_CAUSES_CYCLE, "Connection would cause infinite loop.");
	auto outputConnection =getOutputConnection(output);
	auto inputConnection = toNode->getInputConnection(input);
	makeConnection(outputConnection, inputConnection);
	simulation->invalidatePlan();
}

void Node::connectSimulation(int which) {
	auto outputConnection=getOutputConnection(which);
	auto inputConnection = simulation->getFinalOutputConnection();
	makeConnection(outputConnection, inputConnection);
	simulation->invalidatePlan();
}

void Node::connectProperty(int output, std::shared_ptr<Node> node, int slot) {
	if(doesEdgePreserveAcyclicity(std::static_pointer_cast<Node>(this->shared_from_this()), node) == false) ERROR(Lav_ERROR_CAUSES_CYCLE, "Connection would cause infinite loop.");
	auto &prop = node->getProperty(slot);
	auto conn = prop.getInputConnection();
	if(conn ==nullptr) ERROR(Lav_ERROR_CANNOT_CONNECT_TO_PROPERTY, "Property does not support connections.");
	auto outputConn =getOutputConnection(output);
	makeConnection(outputConn, conn);
	simulation->invalidatePlan();
}

void Node::disconnect(int output, std::shared_ptr<Node> node, int input) {
	auto o =getOutputConnection(output);
	if(node == nullptr) o->clear();
	else {
		auto other = node->getInputConnection(input);
		breakConnection(o, other);
	}
	simulation->invalidatePlan();
}

void Node::isolate() {
	int oc = getOutputConnectionCount();
	for(int i = 0; i < oc; i++) disconnect(i);
}

std::shared_ptr<Simulation> Node::getSimulation() {
	return simulation;
}

Property& Node::getProperty(int slot, bool allowForwarding) {
	//first the forwarded case.
	if(allowForwarding && forwarded_properties.count(slot) !=0) {
		auto n=std::get<0>(forwarded_properties[slot]).lock();
		auto s=std::get<1>(forwarded_properties[slot]);
		if(n) return n->getProperty(s);
	}
	if(properties.count(slot) == 0) ERROR(Lav_ERROR_RANGE, "Invalid property index or identifier.");
	else return properties[slot];
}

void Node::forwardProperty(int ourProperty, std::shared_ptr<Node> toNode, int toProperty) {
	forwarded_properties[ourProperty] = std::make_tuple(toNode, toProperty);
	toNode->addPropertyBackref(toProperty, std::static_pointer_cast<Node>(shared_from_this()), ourProperty);
	simulation->invalidatePlan();
}

void Node::stopForwardingProperty(int ourProperty) {
	if(forwarded_properties.count(ourProperty)) {
		auto t = forwarded_properties[ourProperty];
		forwarded_properties.erase(ourProperty);
		auto n = std::get<0>(t).lock();
		if(n) {
			n->removePropertyBackref(std::get<1>(t), std::static_pointer_cast<Node>(shared_from_this()), ourProperty);
		}
	}
	else ERROR(Lav_ERROR_INTERNAL, "Backref does not exist.");
	simulation->invalidatePlan();
}

void Node::addPropertyBackref(int ourProperty, std::shared_ptr<Node> toNode, int toProperty) {
	forwarded_property_backrefs[ourProperty].insert(std::make_tuple(toNode, toProperty));
}

void Node::removePropertyBackref(int ourProperty, std::shared_ptr<Node> toNode, int toProperty) {
	auto t = std::make_tuple(toNode, toProperty);
	if(forwarded_property_backrefs.count(ourProperty)) {
		if(forwarded_property_backrefs[ourProperty].count(t)) forwarded_property_backrefs[ourProperty].erase(t);
	}
}

void Node::visitPropertyBackrefs(int which, std::function<void(Property&)> pred) {
	for(auto &t: forwarded_property_backrefs[which]) {
		auto &n = std::get<0>(t);
		auto n_s = n.lock();
		if(n_s) {
			auto w = std::get<1>(t);
			//get the property without forwarding.
			pred(n_s->getProperty(w, false));
		}
	}
}

void Node::lock() {
	simulation->lock();
}

void Node::unlock() {
	simulation->unlock();
}

void Node::reset() {
}

//protected resize function.
void Node::resize(int newInputCount, int newOutputCount) {
	int oldInputCount = input_buffers.size();
	for(int i = oldInputCount-1; i >= newInputCount; i--) if(input_buffers[i]) freeArray(input_buffers[i]);
	input_buffers.resize(newInputCount, nullptr);
	for(int i = oldInputCount; i < newInputCount; i++) input_buffers[i] = allocArray<float>(simulation->getBlockSize());

	int oldOutputCount = output_buffers.size();
	if(newOutputCount < oldOutputCount) { //we need to free some arrays.
		for(auto i = newOutputCount; i < oldOutputCount; i++) {
			if(output_buffers[i])
			freeArray(output_buffers[i]);
		}
	}
	//do the resize.
	output_buffers.resize(newOutputCount, nullptr);
	if(newOutputCount > oldOutputCount) { //we need to allocate some more arrays.
		for(auto i = oldOutputCount; i < newOutputCount; i++) {
			output_buffers[i] = allocArray<float>(simulation->getBlockSize());
		}
	}
}

void Node::execute() {
	tick();
}

bool Node::canCull() {
	return getState() == Lav_NODESTATE_PAUSED;
}

//LavSubgraphNode

SubgraphNode::SubgraphNode(int type, std::shared_ptr<Simulation> simulation): Node(type, simulation, 0, 0) {
}

void SubgraphNode::setInputNode(std::shared_ptr<Node> node) {
	subgraph_input= node;
	simulation->invalidatePlan();
}

void SubgraphNode::setOutputNode(std::shared_ptr<Node> node) {
	subgraph_output=node;
	simulation->invalidatePlan();
}

int SubgraphNode::getInputConnectionCount() {
	if(subgraph_input) return subgraph_input->getInputConnectionCount();
	else return 0;
}

std::shared_ptr<InputConnection> SubgraphNode::getInputConnection(int which) {
	if(which < 0|| which >= getInputConnectionCount()) ERROR(Lav_ERROR_RANGE, "Invalid input.");
	else return subgraph_input->getInputConnection(which);
}

int SubgraphNode::getOutputBufferCount() {
	if(subgraph_output) return subgraph_output->getOutputBufferCount();
	else return 0;
}

float** SubgraphNode::getOutputBufferArray() {
	if(subgraph_output) return subgraph_output->getOutputBufferArray();
	return nullptr;
}

//This override is needed because nodes try to add their inputs, but we override where input connections come from.
//In addition, we have no input buffers.
void SubgraphNode::tick() {
	last_processed = simulation->getTickCount();
	//Zeroing the output buffers will silence our output.
	if(getState() == Lav_NODESTATE_PAUSED) {
		zeroOutputBuffers();
		return;
	}
	tickProperties();
	zeroInputBuffers();
	is_processing = true;
	num_input_buffers = input_buffers.size();
	num_output_buffers = output_buffers.size();
	//No process call, subgraphs  don't support it.
	applyMul();
	applyAdd();
	is_processing = false;
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetSimulation(LavHandle handle, LavHandle* destination) {
	PUB_BEGIN
auto n = incomingObject<Node>(handle);
	*destination = outgoingObject(n->getSimulation());
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeConnect(LavHandle nodeHandle, int output, LavHandle destHandle, int input) {
	PUB_BEGIN
	auto node= incomingObject<Node>(nodeHandle);
	auto dest = incomingObject<Node>(destHandle);
	LOCK(*node);
	node->connect(output, dest, input);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeConnectSimulation(LavHandle nodeHandle, int output) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	node->connectSimulation(output);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeConnectProperty(LavHandle nodeHandle, int output, LavHandle otherHandle, int slot) {
	PUB_BEGIN
	auto n = incomingObject<Node>(nodeHandle);
	auto o = incomingObject<Node>(otherHandle);
	LOCK(*n);
	n->connectProperty(output, o, slot);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeDisconnect(LavHandle nodeHandle, int output, LavHandle otherHandle, int input) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	//We have to allow null for this one.
	auto other = incomingObject<Node>(otherHandle, true);
	LOCK(*node);
	node->disconnect(output, other, input);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeIsolate(LavHandle nodeHandle) {
	PUB_BEGIN
	auto n = incomingObject<Node>(nodeHandle);
	LOCK(*n);
	n->isolate();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetInputConnectionCount(LavHandle nodeHandle, unsigned int* destination) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	*destination =node->getInputConnectionCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetOutputConnectionCount(LavHandle nodeHandle, unsigned int* destination) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	*destination = node->getOutputConnectionCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeReset(LavHandle nodeHandle) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	node->reset();
	PUB_END
}

//this is properties.
//this is here because properties do not "know" about objects and only objects have properties; also, it made properties.cpp have to "know" about simulations and objects.

//this works for getters and setters to lock the object and set a variable prop to be a pointer-like thing to a property.
#define PROP_PREAMBLE(n, s, t) auto node_ptr = incomingObject<Node>(n);\
LOCK(*node_ptr);\
auto &prop = node_ptr->getProperty((s));\
if(prop.getType() != (t)) {\
auto _t = prop.getType();\
std::string msg = "Property is a ";\
if(_t == Lav_PROPERTYTYPE_INT) msg+="int";\
else if(_t == Lav_PROPERTYTYPE_FLOAT) msg += "float";\
else if(_t == Lav_PROPERTYTYPE_DOUBLE) msg += "double";\
else if(_t == Lav_PROPERTYTYPE_STRING) msg += "string";\
else if(_t == Lav_PROPERTYTYPE_FLOAT3) msg += "float3";\
else if(_t == Lav_PROPERTYTYPE_FLOAT6) msg += "float6";\
else if(_t == Lav_PROPERTYTYPE_INT_ARRAY) msg += "int array";\
else if(_t == Lav_PROPERTYTYPE_FLOAT_ARRAY) msg += "float array";\
else if(_t == Lav_PROPERTYTYPE_BUFFER) msg += "buffer";\
msg += " property.";\
ERROR(Lav_ERROR_TYPE_MISMATCH, msg);\
}

#define READONLY_CHECK if(prop.isReadOnly()) ERROR(Lav_ERROR_PROPERTY_IS_READ_ONLY, "Attempt to write a read-only property.");

Lav_PUBLIC_FUNCTION LavError Lav_nodeResetProperty(LavHandle nodeHandle, int slot) {
	PUB_BEGIN
	auto node_ptr = incomingObject<Node>(nodeHandle);
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
	auto node= incomingObject<Node>(nodeHandle);
	LOCK(*node);
	auto &prop = node->getProperty(slot);
	*destination = prop.getType();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyName(LavHandle nodeHandle, int slot, char** destination) {
	PUB_BEGIN
	auto node_ptr = incomingObject<Node>(nodeHandle);
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
	auto node = incomingObject<Node>(nodeHandle);
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
	auto ptr = incomingObject<Node>(nodeHandle);
	LOCK(*ptr);
	auto &prop = ptr->getProperty(slot);
	int type = prop.getType();
	if(type != Lav_PROPERTYTYPE_FLOAT_ARRAY || type != Lav_PROPERTYTYPE_INT_ARRAY) ERROR(Lav_ERROR_TYPE_MISMATCH, "Property is not an array.");
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetBufferProperty(LavHandle nodeHandle, int slot, LavHandle bufferHandle) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_BUFFER);
	auto buff=incomingObject<Buffer>(bufferHandle, true);
	if(buff && buff->getSimulation() != node_ptr->getSimulation()) ERROR(Lav_ERROR_CANNOT_CROSS_SIMULATIONS, "Buffer is not from the same simulation as the node.");
	prop.setBufferValue(buff);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetBufferProperty(LavHandle nodeHandle, int slot, LavHandle* destination) {
	PUB_BEGIN
	PROP_PREAMBLE(nodeHandle, slot, Lav_PROPERTYTYPE_BUFFER);
	*destination = outgoingObject(prop.getBufferValue());
	PUB_END
}

}
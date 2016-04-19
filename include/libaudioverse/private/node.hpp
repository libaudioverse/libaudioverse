/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "../libaudioverse.h"
#include "properties.hpp"
#include "memory.hpp"
#include "connections.hpp"
#include "simulation.hpp"
#include <map>
#include <memory>
#include <vector>
#include <set>
#include <utility>
#include "job.hpp"

namespace libaudioverse_implementation {

class Property;
class InputConnection;
class OutputConnection;

//Needed to make comparing property backrefs sane.
class PropertyBackrefComparer {
	public:
	bool operator() (const std::tuple<std::weak_ptr<Node>, int> &a, const std::tuple<std::weak_ptr<Node>, int> &b) const;
};


/**Things all Libaudioverse nodes have.*/
class Node: public Job {
	public:
	Node(int type, std::shared_ptr<Simulation> simulation, unsigned int numInputBuffers, unsigned int numOutputBuffers);
	virtual ~Node();

	virtual int getOutputBufferCount();
	//Note that this isn't shared ptr.  The output pointers for a node are managed by the node itself and we need to be able to allocate/deallocate them for SSE, as well as work with arrays.  Don't hold on to output pointers.
	virtual float** getOutputBufferArray();

	virtual int getInputBufferCount();
	virtual float** getInputBufferArray();

	//equivalent to reading lav_NODE_STATE.
	int getState();
	void setState(int newState);
	void stateChanged();
	
	
	//public view of connections.
	virtual int getInputConnectionCount();
	virtual int getOutputConnectionCount();
	virtual std::shared_ptr<InputConnection> getInputConnection(int which);
	virtual std::shared_ptr<OutputConnection> getOutputConnection(int which);
	//intended to be used by subclasses to add input and output connections.
	virtual void appendInputConnection(int start, int count);
	virtual void appendOutputConnection(int start, int count);

	//make a connection from an output of this node to an input of another.
	virtual void connect(int output, std::shared_ptr<Node> toNode, int input);
	//make a connection from an output of this node to the simulation.
	virtual void connectSimulation(int which);
	//Connects an output to a property.
	virtual void connectProperty(int output, std::shared_ptr<Node> node, int slot);
	//called on an output, this function terminates all connections for which it is involved.
	//If node is null, clear everything.
	virtual void disconnect(int output, std::shared_ptr<Node> node = nullptr, int input = 0);
	//Completely isolate this node.
	//This will be exposed publicly in future, but exists primarily for object deletion: input and output connections must die immediately.
	//If they don't, the planner can see things that are no longer true.
	//Written in terms of connection primitives.
	void isolate();

	//Ticks all properties.
	virtual void tickProperties();
	//do not override. Handles the processing protocol (updating some globals and calling process) if needed for this tick, otherwise does nothing.
	virtual void tick();
	//override this one instead. Default implementation merely zeros the outputs.
	virtual void process();
	//Apply mul and add.
	virtual void applyMul();
	virtual void applyAdd();
	//zero the output buffers.
	virtual void zeroOutputBuffers();
	virtual void zeroInputBuffers();
	//Does some cleanup and the like.
	//This is also an override point for subclasses that may need to do cleanup periodically in order to remain performant; in that case, they *must* call the base. Or else.
	virtual void doMaintenance();
	//Called after the simulation is locked but before ticking, in some arbetrary order. We must be registered for this.
	//It is safe to change connections here.
	virtual void willTick();
	
	std::shared_ptr<Simulation> getSimulation();
	Property& getProperty(int slot, bool allowForwarding = true);

	//Property forwarding support.
	void forwardProperty(int ourProperty, std::shared_ptr<Node> toNode, int toProperty);
	void stopForwardingProperty(int ourProperty);
	//Record that toProperty on toNode is forwarded to us.
	void addPropertyBackref(int ourProperty, std::shared_ptr<Node> toNode, int toProperty);
	void removePropertyBackref(int ourProperty, std::shared_ptr<Node> toNode, int toProperty);
	//call pred on all the properties that immediately forward to which.
	void visitPropertyBackrefs(int which, std::function<void(Property&)> pred);


	//meet the lockable concept.
	//Warning: these aren't virtual because they're just so that our macro works; all locking still forwards to devices.
	void lock();
	void unlock();

	//Override hook for resetting.
	virtual void reset();

	//change number of input and output buffers.
	virtual void resize(int newInputCount, int newOutputCount);

	//Conform to Job.
	virtual void execute();

	//True if we're paused.
	bool canCull() override;
	
	//Various optimizations that subclasses can enable.
	void setShouldZeroOutputBuffers(bool v);
	protected:
	std::shared_ptr<Simulation> simulation = nullptr;
	std::map<int, Property> properties;
	//the tuple is of (node, property).
	std::map<int, std::tuple<std::weak_ptr<Node>, int>> forwarded_properties;
	//These are the back references, used for property callbacks.
	std::map<int, std::set<std::tuple<std::weak_ptr<Node>, int>, PropertyBackrefComparer>> forwarded_property_backrefs;
	
	std::vector<float*> input_buffers;
	std::vector<float*> output_buffers;
	std::vector<std::shared_ptr<InputConnection>> input_connections;
	std::vector<std::shared_ptr<OutputConnection>> output_connections;
	bool is_processing = false, is_suspended = false;
	int num_input_buffers = 0, num_output_buffers = 0, block_size = 0;
	//used to make no-op state changes free.
	int prev_state = Lav_NODESTATE_PLAYING;
	int last_processed = -1; //-1 so that it's not equal to the simulation's tick counter, which starts at 0.

	//we are never allowed to copy.
	Node(const Node&) = delete;
	Node& operator=(const Node&) = delete;
	
	//various optimization flags.
	bool should_zero_output_buffers = true; //Enable/disable zeroing output buffers on tick if node is unpaused.
	template<typename JobT, typename CallableT, typename... ArgsT>
	friend void nodeVisitDependencies(JobT&& start, CallableT&& callable, ArgsT&&... args);
};

/**This is the creation template for a node.
Every createXXX function uses this template.*/
template<typename NodeT, typename... ArgsT>
std::shared_ptr<NodeT> standardNodeCreation(std::shared_ptr<Simulation> simulation, ArgsT... args) {
	std::shared_ptr<NodeT> ret(new NodeT(simulation, std::forward<ArgsT>(args)...), ObjectDeleter(simulation));
	simulation->associateNode(ret);
	return ret;
}

}
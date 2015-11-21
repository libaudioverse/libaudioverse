/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../libaudioverse.h"
#include "properties.hpp"
#include "memory.hpp"
#include "connections.hpp"
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
	
	template<typename JobT, typename CallableT, typename... ArgsT>
	friend void nodeVisitDependencies(JobT&& start, CallableT&& callable, ArgsT&&... args);
};

/*needed for things that wish to encapsulate and manage nodes that the public API isn't supposed to see.
Usage: append output connections as normal and configure as normal.
The subgraph node forwards most calls onto the current output object, including those for getting output arrays. Consequently, the subgraph node has the same number of output buffers as the output object-and the output object may be changed.
The properties mul and (todo) add are forwarded onto the output node before every block.
Changing the input node is defined behavior: it will break horribly and unpredictably.
Changing the output node is safe so long as the connections on the subgraph are reconfigured, same as for any other resize.*/
class SubgraphNode: public Node {
	public:
	SubgraphNode(int type, std::shared_ptr<Simulation> simulation);
	virtual void setInputNode(std::shared_ptr<Node> node);
	virtual void setOutputNode(std::shared_ptr<Node> node);
	//these all forward onto the input node.
	int getInputConnectionCount() override;
	std::shared_ptr<InputConnection> getInputConnection(int which) override;
	//these forward onto the output node, making connections to the subgraph magically work.
	int getOutputBufferCount() override;
	float** getOutputBufferArray() override;

	//Override tick because we can't try to use connections.
	//We don't have proper input buffers, default tick will override who knows what.
	void tick() override;
	
	protected:
	std::shared_ptr<Node> subgraph_input, subgraph_output;
	
	template<typename JobT, typename CallableT, typename... ArgsT>
	friend void subgraphNodeVisitDependencies(JobT&& start, CallableT &&callable, ArgsT&&... args);
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
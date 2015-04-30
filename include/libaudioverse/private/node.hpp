/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../libaudioverse.h"
#include "properties.hpp"
#include "events.hpp"
#include "connections.hpp"
#include "memory.hpp"
#include <map>
#include <memory>
#include <vector>
#include <set>

class Property;

/**Things all Libaudioverse nodes have.*/
class Node: public ExternalObject { //enable_shared_from_this is for event infrastructure.
	public:
	Node(int type, std::shared_ptr<Simulation> simulation, unsigned int numInputBuffers, unsigned int numOutputBuffers);
	virtual ~Node();

	virtual int getOutputBufferCount();
	//Note that this isn't shared ptr.  The output pointers for a node are managed by the node itself and we need to be able to allocate/deallocate them for SSE, as well as work with arrays.  Don't hold on to output pointers.
	virtual float** getOutputBufferArray();

	virtual int getInputBufferCount();
	virtual float** getInputBufferArray();

	//equivalent to reading lav_NODE_STATE.
	virtual int getState();

	//public view of connections.
	virtual int getInputConnectionCount();
	virtual int getOutputConnectionCount();
	//these next two return shared pointers which "alias" this object.
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
	virtual void disconnect(int which);

	//Ticks all properties.
	virtual void tickProperties();
	//do not override. Handles the processing protocol (updating some globals and calling process) if needed for this tick, otherwise does nothing.
	virtual void tick();
	//override this one instead. Default implementation merely zeros the outputs.
	virtual void process();
	//zero the output buffers.
	virtual void zeroOutputBuffers();
	virtual void zeroInputBuffers();
	//Does some cleanup and the like.
	//This is also an override point for subclasses that may need to do cleanup periodically in order to remain performant; in that case, they *must* call the base. Or else.
	virtual void doMaintenance();
	//this is called at some point in the processing logic which is guaranteed to be before this node's parents are processed and after the device is locked.
	//additionally, a parent will have its willProcessParents called after this node.
	//that is, nothing else will touch this node but the mixer thread, and the next thing to be called (at some point in future) is willProcess.
	//the default does nothing.
	virtual void willProcessParents();

	virtual std::shared_ptr<Simulation> getSimulation();
	virtual Property& getProperty(int slot);

	//event helper methods.
	Event& getEvent(int which);

	//meet the lockable concept.
	//Warning: these aren't virtual because they're just so that our macro works; all locking still forwards to devices.
	void lock();
	void unlock();

	//Override hook for resetting.
	virtual void reset();

	//change number of input and output buffers.
	virtual void resize(int newInputCount, int newOutputCount);

	//Return a set containing all nodes upon which we depend.
	std::set<std::shared_ptr<Node>> getDependencies();
	protected:
	std::shared_ptr<Simulation> simulation = nullptr;
	std::map<int, Property> properties;
	std::map<int, Event> events;
	std::vector<float*> input_buffers;
	std::vector<float*> output_buffers;
	std::vector<InputConnection> input_connections;
	std::vector<OutputConnection> output_connections;
	bool is_processing = false, is_suspended = false;
	int num_input_buffers = 0, num_output_buffers = 0, block_size = 0;
	//used to make no-op state changes free.
	int prev_state = Lav_NODESTATE_PLAYING;
	int last_processed = -1; //-1 so that it's not equal to the simulation's tick counter, which starts at 0.

	//we are never allowed to copy.
	Node(const Node&) = delete;
	Node& operator=(const Node&) = delete;
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

	//this override is to make processing work properly. We must do a willProcessParents on ourselves and then forward to the output, if any.
	void tick() override;

	protected:
	std::shared_ptr<Node> subgraph_input, subgraph_output;
};

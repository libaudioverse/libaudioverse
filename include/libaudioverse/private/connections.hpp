/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <memory>
#include <set>
#include <vector>

class Node;
class Simulation;

/**Overview:

These two classes implement the connection logic for Libaudioverse: the ability to pass more than one channel of audio around, upmixing and dowmixing in all common cases.

Nodes have any number of input connections and any number of output connections; these are exposed publicly as simply inputs and outputs.  When nodes die, connections automatically break.

Shared pointers geta bit funny here: the shared pointer that has to be used must come from and share ownership with a Node shared pointer.
To that end, connection logic is contained in a function that favours neither class, and some functions (specifically those intended to be called on the other class and mostly/completely private to this module) use raw pointers to indicate which object they're talking about.*/

//we need weak pointers to this in OutputConnection.
class InputConnection;

class OutputConnection {
	public:
	//start: index of the output buffer at which this connection begins.
	//count: the number of adjacent output buffers to which this connection applies.
	OutputConnection(std::shared_ptr<Simulation> simulation, Node* node, int start, int count);
	void add(int inputBufferCount, float** inputBuffers, bool shouldApplyMixingMatrix);
	void reconfigure(int newStart, int newCount);
	void clear();
	void connectHalf(std::shared_ptr<InputConnection> inputConnection);
	int getStart() {return start;}
	int getCount() {return count;}
	Node* getNode();
	std::vector<Node*> getConnectedNodes();
	private:
	Node* node = nullptr;
	int start, count;
	std::set<std::weak_ptr<InputConnection>, std::owner_less<std::weak_ptr<InputConnection>>> connected_to;
	std::shared_ptr<Simulation> simulation;
};

/**Unlike output connections, input connections may have a null node, so long as the nodeless functions are used.
this is to prevent needing to make special case code for simulations.*/
class InputConnection {
	public:
	InputConnection(std::shared_ptr<Simulation> simulation, Node* node, int start, int count);
	void add(bool applyMixingMatrix); //calls out to the output connections this owns, no further parameters are needed.
	void addNodeless(float** inputs, bool shouldApplyMixingMatrix);
	void reconfigure(int start, int count);
	void connectHalf(std::shared_ptr<OutputConnection>outputConnection);
	void forgetConnection(OutputConnection* which);
	int getStart() {return start;}
	int getCount() {return count;}
	Node* getNode();
	std::vector<Node*> getConnectedNodes();
	private:
	Node* node;
	int start, count;
	std::set<std::weak_ptr<OutputConnection>, std::owner_less<std::weak_ptr<OutputConnection>>> connected_to;
	std::shared_ptr<Simulation> simulation;
};

void makeConnection(std::shared_ptr<OutputConnection> output, std::shared_ptr<InputConnection> input);

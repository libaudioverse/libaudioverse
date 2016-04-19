/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include <memory>
#include <set>
#include <vector>

namespace libaudioverse_implementation {

class Node;
class Simulation;

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
	void disconnectHalf(std::shared_ptr<InputConnection> connection);
	int getStart() {return start;}
	int getCount() {return count;}
	Node* getNode();
	std::vector<Node*> getConnectedNodes();
	private:
	Node* node = nullptr;
	int start, count, block_size;
	std::set<std::weak_ptr<InputConnection>, std::owner_less<std::weak_ptr<InputConnection>>> connected_to;
};

/**Unlike output connections, input connections may have a null node, so long as the nodeless functions are used.
this is to prevent needing to make special case code for simulations.

Input connections keep nodes alive.*/
class InputConnection {
	public:
	InputConnection(std::shared_ptr<Simulation> simulation, Node* node, int start, int count);
	void add(bool applyMixingMatrix); //calls out to the output connections this owns, no further parameters are needed.
	void addNodeless(float** inputs, bool shouldApplyMixingMatrix);
	void reconfigure(int start, int count);
	void connectHalf(std::shared_ptr<OutputConnection>outputConnection);
	void disconnectHalf(std::shared_ptr<OutputConnection> connection);
	void forgetConnection(OutputConnection* which);
	int getStart() {return start;}
	int getCount() {return count;}
	Node* getNode();
	std::vector<Node*> getConnectedNodes();
	int getConnectedNodeCount();
	
	template<typename CallableT, typename... ArgsT>
	void visitInputs(CallableT&& callable, ArgsT&&... args) {
		for(auto &i: connected_to) callable(i.second, args...);
	}
	private:
	Node* node;
	int start, count, block_size;
	std::map<std::shared_ptr<OutputConnection>, std::shared_ptr<Node>> connected_to;
};

void makeConnection(std::shared_ptr<OutputConnection> output, std::shared_ptr<InputConnection> input);
void breakConnection(std::shared_ptr<OutputConnection> output, std::shared_ptr<InputConnection> input);


}
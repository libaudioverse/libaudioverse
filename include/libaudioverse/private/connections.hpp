/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include <memory>
#include <set>
#include <vector>

namespace libaudioverse_implementation {

class Node;
class Server;

//we need weak pointers to this in OutputConnection.
class InputConnection;

class OutputConnection {
	public:
	//start: index of the output buffer at which this connection begins.
	//count: the number of adjacent output buffers to which this connection applies.
	OutputConnection(std::shared_ptr<Server> server, Node* node, int start, int count);
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
this is to prevent needing to make special case code for server.

Input connections keep nodes alive.*/
class InputConnection {
	public:
	InputConnection(std::shared_ptr<Server> server, Node* node, int start, int count);
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
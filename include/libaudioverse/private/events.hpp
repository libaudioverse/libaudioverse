/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <utility>
#include <memory>
#include <string>
#include <atomic>
#include <functional>

namespace libaudioverse_implementation {

class Simulation;
class Node;

class Event {
	public:
	Event();
	Event(const Event& other);
	Event& operator=(const Event other);
	void setHandler(std::function<void(std::shared_ptr<Node>, void*)> cb);
	//these two are for when using externally. Allows us to make a query of what the handler is for c api.
	void setExternalHandler(LavEventCallback cb);
	LavEventCallback getExternalHandler();
	void fire();
	void associateSimulation(std::shared_ptr<Simulation> sim);
	void associateNode(Node* node);
	const char* getName();
	void setName(const char* n);
	void* getUserData();
	void setUserData(void* data);
	bool getNoMultifire();
	void setNoMultifire(bool what);
	//this has to be here.
	friend void swap(Event &a, Event &b) {
		using std::swap;
		swap(a.associated_simulation, b.associated_simulation);
		swap(a.handler, b.handler);
		swap(a.name, b.name);
		swap(a.associated_node, b.associated_node);
		swap(a.user_data, b.user_data);
		swap(a.no_multifire, b.no_multifire);
		//ignore isFiring.
	};
	private:
	std::shared_ptr<Simulation> associated_simulation = nullptr;
	LavEventCallback external_handler = nullptr;
	std::function<void(std::shared_ptr<Node>, void*)> handler;
	std::string name;
	Node* associated_node = nullptr;
	void* user_data = nullptr;
	std::atomic<int> is_firing;
	bool no_multifire = false;
};

}
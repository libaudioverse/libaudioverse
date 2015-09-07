/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/events.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <string>
#include <memory>

namespace libaudioverse_implementation {

/**events.*/

Event::Event() {
	is_firing.store(0);
}

Event::Event(const Event &other):
name(other.name), associated_simulation(other.associated_simulation), handler(other.handler), associated_node(other.associated_node), user_data(other.user_data), no_multifire(other.no_multifire) {
	is_firing.store(0);
}

Event& Event::operator=(Event other) {
	swap(*this, other);
	return *this;
}

void Event::setExternalHandler(LavEventCallback cb) {
	external_handler = cb;
}

LavEventCallback Event::getExternalHandler() {
	return external_handler;
}

void Event::setHandler(std::function<void(std::shared_ptr<Node>, void*)> handler) {
	this->handler = handler;
}

void Event::fire() {
	if(handler == false) return; //nothing to do.
	if(no_multifire && is_firing.load() == 1) return;
	is_firing.store(1);
	//we need to hold local copies of both the node and data in case they are changed between firing and processing by the simulation.
	auto node= std::weak_ptr<Node>(std::static_pointer_cast<Node>(associated_node->shared_from_this()));
	void* userdata = user_data;
	//fire a lambda that uses these by copy.
	associated_simulation->enqueueTask([=]() {
		auto shared_node= node.lock();
		if(shared_node== nullptr) return;	
		//callbacks die with nodes; if we get this far, this is still a valid pointer.
		handler(shared_node, userdata);
		this->is_firing.store(0);
	});
}

void Event::associateSimulation(std::shared_ptr<Simulation> simulation) {
	associated_simulation = simulation;
}

void Event::associateNode(Node* node) {
	associated_node= node;
}

const char* Event::getName() {
	return name.c_str();
}

void Event::setName(const char* n) {
	name = std::string(n);
}

void* Event::getUserData() {
	return user_data;
}

void Event::setUserData(void* data) {
	user_data = data;
}

bool Event::getNoMultifire() {
	return no_multifire;
}

void Event::setNoMultifire(bool what) {
	no_multifire = what;
}

}
/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <utility>
#include <memory>
#include <string>
#include <atomic>
#include <functional>

class LavSimulation;

class LavEvent {
	public:
	LavEvent();
	LavEvent(const LavEvent& other);
	LavEvent& operator=(const LavEvent other);
	void setHandler(std::function<void(LavNode*, void*)> cb);
	//these two are for when using externally. Allows us to make a query of what the handler is for c api.
	void setExternalHandler(LavEventCallback cb);
	LavEventCallback getExternalHandler();
	void fire();
	void associateSimulation(std::shared_ptr<LavSimulation> sim);
	void associateNode(LavNode* node);
	const char* getName();
	void setName(const char* n);
	void* getUserData();
	void setUserData(void* data);
	bool getNoMultifire();
	void setNoMultifire(bool what);
	//this has to be here.
	friend void swap(LavEvent &a, LavEvent &b) {
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
	std::shared_ptr<LavSimulation> associated_simulation = nullptr;
	LavEventCallback external_handler = nullptr;
	std::function<void(LavNode*, void*)> handler;
	std::string name;
	LavNode* associated_node = nullptr;
	void* user_data = nullptr;
	std::atomic<int> is_firing;
	bool no_multifire = false;
};
/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/planner.hpp>
#include <vector>
#include <memory>
#include <algorithm>

namespace libaudioverse_implementation {

Planner::Planner() {
}

Planner::~Planner() {
}

void Planner::execute(std::shared_ptr<Job> start) {
	std::set<std::shared_ptr<Job>> seen;
	std::function<void(std::shared_ptr<Job>, int)> visitor; //needs to be a function object for recursion.
	visitor = [&](std::shared_ptr<Job> j, int t) {
		if(j == nullptr) return;
		t = std::min(j->job_sort_tag, t); //First job is 0 for sorting purposes, and we proceed negatively.
		j->job_sort_tag = t;
		seen.insert(j);
		//Visit dependencies.
		j->visitDependencies([&] (std::shared_ptr<Job> j2) {visitor(j2, t-1);});
	};
	//Call the first visitor:
	visitor(start, 0);
	plan.assign(seen.begin(), seen.end());
	//sort the vector.
	//Since the deepest jobs are negative, this works.
	std::sort(plan.begin(), plan.end(),
	[](const std::shared_ptr<Job> &a, const std::shared_ptr<Job> &b)->bool {return a->job_sort_tag < b->job_sort_tag;});
	//Execute from right to left  and then from left to right.
	for(auto i = plan.rbegin(); i != plan.rend(); i++) {
		(*i)->willExecuteDependencies();
	}
	for(auto i = plan.begin(); i != plan.end(); i++) {
		(*i)->execute();
	}
	//Kill the shared pointers.
	plan.clear();
}

}
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

//Small helper  function, which needn't know about the class (thus avoiding capture requirements).
void tagger(std::shared_ptr<Job> job, int tag, std::vector<std::shared_ptr<Job>> &destination) {
	tag = std::min(tag, job->job_sort_tag);
	job->job_sort_tag = tag;
	destination.push_back(job);
	job->visitDependencies([&](std::shared_ptr<Job> j) {tagger(j, tag-1, destination);});
}

void Planner::execute(std::shared_ptr<Job> start) {
	//Fill the vector with the jobs.
	tagger(start, 0, plan);
	//In the common case, the vector is sorted by a reverse.
	std::reverse(plan.begin(), plan.end());
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

void Planner::invalidatePlan() {
}

}
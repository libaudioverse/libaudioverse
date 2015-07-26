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

void Planner::execute(std::shared_ptr<Job> start, int threads) {
	if(is_valid == false || last_start.lock() != start) replan(start);
	plan.assign(weak_plan.begin(), weak_plan.end());
	//Check it for nulls.  This is theoretically possible.
	for(auto &p: plan) {
		if(p == nullptr) {
			invalidatePlan();
			plan.clear();
			//recurse into ourselves and try to plan again.
			execute(start, threads);
		}
	}
	for(auto i = plan.rbegin(); i != plan.rend(); i++) {
		(*i)->willExecuteDependencies();
	}
	if(threads == 1) {
		runJobsSync();
	}
	else {
		if(last_thread_count != threads) {
			thread_pool.setThreadCount(threads);
			last_thread_count = threads;
		}
		if(started_thread_pool == false) {
			thread_pool.start();
			started_thread_pool = true;
		}
		runJobsAsync();
	}
	//Kill the shared pointers.
	plan.clear();
	last_start = start;
}

void Planner::runJobsSync() {
	for(auto i = plan.begin(); i != plan.end(); i++) {
		(*i)->execute();
		(*i)->job_recorded = false; //clear for the next time we plan.
	}
}

void Planner::runJobsAsync() {
	if(plan.size() == 0) return; //nothing to do.
	int prev_tag = plan[0]->job_sort_tag;
	auto i = plan.begin();
	auto end = plan.end();
	//Run through it, enqueueing jobs and barriers.
	while(i != end) {
		while(i != end && (*	i)->job_sort_tag == prev_tag) { //While jobs are guaranteed to not depend on each other.
			thread_pool.submitJob([i] () {
				auto jobPtr = *i;
				jobPtr->execute();
				jobPtr->job_recorded = false;
			});
			i++;
		}
		//Make sure everything finishes before we start the next ones.
		thread_pool.submitBarrier();
		if(i != end) prev_tag = (*i)->job_sort_tag;
	}
	//At this point, submit a meaningless job that does nothing.
	//This lets us synchronize with the end of this batch.
	auto future = thread_pool.submitJobWithResult([](){});
	future.wait();
	//And that's it.
}

void Planner::invalidatePlan() {
	is_valid = false;
}

//Actually do the planning below here:
//Small helper  function, which needn't know about the class (thus avoiding capture requirements).
void tagger(std::shared_ptr<Job> job, int tag, std::vector<std::shared_ptr<Job>> &destination) {
	tag = std::min(tag, job->job_sort_tag);
	//If we are not changing the tag and this job was seen, we can abort the recursion early;
	//In such a case, we will not change anything.
	bool skipRecursion = (job->job_sort_tag == tag && job->job_recorded);
	job->job_sort_tag = tag;
	if(job->job_recorded == false) {
		destination.push_back(job);
		job->job_recorded = true;
	}
	if(skipRecursion == false) {
		std::function<void(std::shared_ptr<Job>&)> f = [&](std::shared_ptr<Job> &j) {tagger(j, tag-1, destination);};
		job->visitDependencies(f);
	}
}

bool jobComparer(const std::shared_ptr<Job> &a, const std::shared_ptr<Job> &b) {
	return a->job_sort_tag < b->job_sort_tag;
}

void Planner::replan(std::shared_ptr<Job> start) {
	//Fill the vector with the jobs.
	tagger(start, 0, plan);
	//In the common case, the vector is sorted by a reverse.
	std::reverse(plan.begin(), plan.end());
	//sort the vector.
	//Since the deepest jobs are negative, this works.
	std::sort(plan.begin(), plan.end(), jobComparer);
	is_valid = true;
	//Put in weak_plan, the cache.
	weak_plan.assign(plan.begin(), plan.end());
	plan.clear();
}

}
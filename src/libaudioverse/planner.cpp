/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/planner.hpp>
#include <libaudioverse/private/audio_thread.hpp>
#include <libaudioverse/private/logging.hpp>
#include <libaudioverse/private/dependency_computation.hpp>
#include <libaudioverse/private/helper_templates.hpp>
#include <vector>
#include <memory>
#include <algorithm>

namespace libaudioverse_implementation {

Planner::Planner() {
}

Planner::~Planner() {
}

void Planner::execute(std::shared_ptr<Job> start, int threads) {
	if(last_start.lock() != start) invalidatePlan();
	if(is_valid == false) replan(start);
	else initializeStrongPlan(); //Try to get it from the cache.
	//We might invalidate because of a dead weak pointer, but this can only happen once.
	if(is_valid == false) replan(start);
	if(threads == 1) {
		runJobsSync();
	}
	else {
		if(started_thread_pool == false) {
			thread_pool.setThreadCount(threads);
			thread_pool.start();
			started_thread_pool = true;
			last_thread_count = threads;
		}
		if(last_thread_count != threads) {
			thread_pool.setThreadCount(threads);
			last_thread_count = threads;
		}
		runJobsAsync();
	}
	clearStrongPlan();
	last_start = start;
}

void jobExecutor(std::shared_ptr<Job> &j) {
	j->execute();
	j->job_recorded = false;
}

void Planner::runJobsSync() {
	becomeAudioThread();
	for(auto &bin: plan) {
		for(auto &j: bin.second) {
			jobExecutor(j);
		}
	}
	//We are potentially sharing this thread with someone else. It is important that we don't accidentally give them high priority too.
	unbecomeAudioThread();
}

void Planner::runJobsAsync() {
	//becomeAudioThread is no-op if called multiple times.
	//Putting it here greatly simplifies thread pool startup logic.
	thread_pool.submitJobToAllThreads(becomeAudioThread);
	for(auto &bin: plan) {
		thread_pool.map(jobExecutor, bin.second.begin(), bin.second.end());
		thread_pool.submitBarrier();
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
inline void binner(std::shared_ptr<Job> job, int tag, std::map<int, std::vector<std::shared_ptr<Job>>> &destination) {
	//if the job is recorded or cullable, we can skip out.
	if(job->job_recorded || job->canCull()) return;
	//Visit our dependencies first.  We want this to make sure we're in the lowest bin we have to be in.
	//Consider a graph, a->b, a->c->b.
	//If we're called on b as a's dependency and record, then it won't happen before c.
	visitDependencies(job, binner, tag-1, destination);
	//The job may have just been recorded.  if it was, we can abort.
	if(job->job_recorded) return;
	//We know it can't be culled because this never changes. So put it in.
	destination[tag].emplace_back(job);
	job->job_recorded = true;
}

void Planner::replan(std::shared_ptr<Job> start) {
	logDebug("Replanning.");
	//Fill the vector with the jobs.
	binner(start, 0, plan);
	is_valid = true;
	//Put in weak_plan, the cache.
	//We do two loops because we really don't want to keep deleting and recreating the vectors.
	//First loop: kill bins in the weak plan that have no correspondance anymore.
	filter(weak_plan, [](decltype(weak_plan)::value_type &i, decltype(plan) &plan)->bool {
		return plan.count(i.first);
	}, plan);
	for(auto &bin: plan) {
		weak_plan[bin.first].assign(bin.second.begin(), bin.second.end());
	}
}

void Planner::clearStrongPlan() {
	for(auto &bin: plan) bin.second.clear();
}

void Planner::initializeStrongPlan() {
	for(auto &bin: weak_plan) {
		plan[bin.first].assign(bin.second.begin(), bin.second.end());
		for(auto &j: plan[bin.first]) if(j == nullptr) {
			invalidatePlan();
			return;
		}
	}
}

}
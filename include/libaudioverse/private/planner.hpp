/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <functional>
#include <set>
#include <vector>
#include <memory>
#include <powercores/thread_pool.hpp>
#include "job.hpp"

/**job.hpp contains the rest of this code.*/

namespace libaudioverse_implementation {
class Planner {
	public:
	Planner();
	~Planner();
	
	//These three functions make up the planning logic; the entry point is execute.
	//Threads must be greater than 0.	
	void execute(std::shared_ptr<Job> start, int threads = 1);
	void runJobsSync();
	void runJobsAsync();
	
	void invalidatePlan();
	private:
	void replan(std::shared_ptr<Job> start);
	//After every tick, kill the shared pointers so that we can let things die.
	void clearStrongPlan();
	//Initialize the strong version of the plan from the weak pointers.
	//This can invalidate the plan.
	void initializeStrongPlan();
	std::map<int, std::vector<std::shared_ptr<Job>>> plan;
	std::map<int, std::vector<std::weak_ptr<Job>>> weak_plan;
	bool is_valid = false;
	std::weak_ptr<Job> last_start;
	//For threads:
	bool started_thread_pool = false;
	int last_thread_count = 0;
	powercores::ThreadPool thread_pool{0};
};

}
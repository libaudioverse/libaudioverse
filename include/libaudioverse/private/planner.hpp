/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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
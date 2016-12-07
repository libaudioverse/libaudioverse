/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include "memory.hpp"
#pragma once
#include <set>
#include <vector>
#include <map>
#include <memory>

/**See planner.hpp.
This file is for the Job base class, and reduces dependencies on powercores.*/

namespace libaudioverse_implementation {

class Job;

//Places jobs into their bins.
void binner(std::shared_ptr<Job> job, int tag, std::map<int, std::vector<std::shared_ptr<Job>>> &destination);
/**Compares smart pointers to jobs: terue if job a comes-before job b.
bool jobComparer(const std::shared_ptr<Job> &a, const std::shared_ptr<Job> &b);

/**Represents a repeatable unit of work.*/
class  Job: public ExternalObject {
	public:
	Job(int type): ExternalObject(type) {}
	virtual ~Job() {}
	virtual void execute() {}
	virtual bool canCull() {return false;}
	private:
	bool job_recorded = false;
	friend void binner(std::shared_ptr<Job> job, int tag, std::map<int, std::vector<std::shared_ptr<Job>>> &destination);
	friend class Planner;
	friend void jobExecutor(std::shared_ptr<Job> &j); //Used by the planner to run jobs.
};

}
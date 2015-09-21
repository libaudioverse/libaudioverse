/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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
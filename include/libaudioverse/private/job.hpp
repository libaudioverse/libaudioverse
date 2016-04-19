/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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
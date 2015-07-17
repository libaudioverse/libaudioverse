/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <functional>
#include <set>
#include <vector>
#include <memory>

namespace libaudioverse_implementation {

class Job;

/**Tags jobs with a number indicating when theya re to run.*/
void tagger(std::shared_ptr<Job> job, int tag, std::vector<std::shared_ptr<Job>> &destination);

/**Represents a repeatable unit of work.*/
class  Job {
	public:
	virtual ~Job() {}
	//Call pred on all dependent jobs.
	//It's okay to visit a dependency twice, the planners protect against this.
	virtual void visitDependencies(std::function<void(std::shared_ptr<Job>)> pred) = 0;
	//Override points for executing.
	virtual void willExecuteDependencies() {}
	virtual void execute() {}
	friend class Planner;
	private:
	int job_sort_tag = 0;
	friend void tagger(std::shared_ptr<Job> job, int tag, std::vector<std::shared_ptr<Job>> &destination);
};

class Planner {
	public:
	Planner();
	~Planner();
	void execute(std::shared_ptr<Job> start);
	private:
	std::vector<std::shared_ptr<Job>> plan;
};

}
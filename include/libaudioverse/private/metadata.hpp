/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <map>
#include "properties.hpp"

namespace libaudioverse_implementation {

void initializeMetadata();
std::map<int, Property> makePropertyTable(int objtype);
const char* getGitRevision();
const char* getCompilerCFlags();
const char* getCompilerCxxFlags();
const char* getBuildType();
const char* getLinkerFlags();

}
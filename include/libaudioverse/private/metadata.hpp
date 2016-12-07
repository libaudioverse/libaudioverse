/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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
/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Precomputed function tables.*/

#include <stdlib.h>
#include <string.h>
#include <libaudioverse/private/functiontables.hpp>
#include <math.h>

namespace libaudioverse_implementation {

/**As a property of these tables, the last sample of the array must be the same as the first, and length must not include the last sample.

Justification: this greatly simplifies interpolating between samples when the end of the array is coming up, completely removing some operations and simplifying weight calculations.*/

void initializeFunctionTables() {
}

}
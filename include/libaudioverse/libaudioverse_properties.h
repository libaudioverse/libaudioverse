#pragma once
#include "libaudioverse.h"

/**This file contains the property enums for Libaudioverse.

It is worth keeping separate because it will grow rapidly and contain documentation comments and etc.

The standard, with the exception of the two standard properties, is:
Lav_NODETYPE_PROPNAME
*/

enum Lav_STDPROPERTIES {
	Lav_STDPROPERTY_ADD = 0,
	Lav_STDPROPERTY_MUL = 1,
};

enum Lav_SINE_PROPERTIES {
	Lav_SINE_FREQUENCY = 0,
};


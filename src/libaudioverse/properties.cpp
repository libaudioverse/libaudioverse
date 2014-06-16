/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_properties.hpp>
#include <stdlib.h>
#include <string.h>

/*Note: having properties be very very very fast is important, so consequently go see the header file.*/

LavProperty* makeIntProperty(int default, int min, int max) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_INT);
	retval->setIntDefault(default);
	retval->setIntRange(min, max);
	retval->reset();
	return retval;
}

LavProperty* makeFloatProperty(float default, float min, float max) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_FLOAT);
	retval->setFloatDefault(default);
	retval->setFloatRange(min, max);
	retval->reset();
	return retval;
}

LavProperty* makeDoubleProperty(double default, double min, double max) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_DOUBLE);
	retval->setDoubleDefault(default);
	retval->setDoubleRange(min, max);
	retval->reset();
	return retval;
}

LavProperty* makeFloat3Property(float default[3]) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_FLOAT3);
	retval->setFloat3Default(v);
	retval->reset();
	return retval;
}

LavProperty* makeFloat6Property(float v[6]) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_FLOAT6);
	retval->setFloat6Default(v);
	retval->reset();e
	return retval;
}	

LavProperty* makeStringProperty(std::string default) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_STRING);
	retval->setStringDefault(default);
	retval->reset();
	return retval;
}

/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_properties.hpp>

#include <stdlib.h>
#include <string.h>

/*Note: having properties be very very very fast is important, so consequently go see the header file.*/

LavProperty* createIntProperty(const char* name, int default, int min, int max) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_INT);
	retval->setIntDefault(default);
	retval->setIntRange(min, max);
	retval->setName(name);
	retval->reset();
	return retval;
}

LavProperty* createFloatProperty(const char* name, float default, float min, float max) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_FLOAT);
	retval->setName(name);
	retval->setFloatDefault(default);
	retval->setFloatRange(min, max);
	retval->reset();
	return retval;
}

LavProperty* createDoubleProperty(const char* name, double default, double min, double max) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_DOUBLE);
	retval->setDoubleDefault(default);
	retval->setDoubleRange(min, max);
	retval->setName(name);
	retval->reset();
	return retval;
}

LavProperty* createFloat3Property(const char* name, float default[3]) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_FLOAT3);
	retval->setFloat3Default(default);
	retval->setName(name);
	retval->reset();
	return retval;
}

LavProperty* createFloat6Property(const char* name, float v[6]) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_FLOAT6);
	retval->setFloat6Default(v);
	retval->setName(name);
	retval->reset();
	return retval;
}	

LavProperty* createStringProperty(const char* name, const char* default) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_STRING);
	retval->setStringDefault(default);
	retval->setName(name);
	retval->reset();
	return retval;
}

LavProperty* createIntArrayProperty(const char* name, unsigned int minLength, unsigned int maxLength, unsigned int defaultLength, int* defaultData) {
	auto prop = new LavProperty(Lav_PROPERTYTYPE_INT_ARRAY);
	prop->setArrayLengthRange(minLength, maxLength);
	std::vector<int> new_default;
	new_default.resize(defaultLength);
	std::copy(defaultData, defaultData+defaultLength, new_default.begin());
	prop->setIntArrayDefault(new_default);
	prop->setName(name);
	prop->reset();	
	return prop;
}

LavProperty* createFloatArrayProperty(const char* name, unsigned int minLength, unsigned int maxLength, unsigned int defaultLength, float* defaultData) {
	auto prop = new LavProperty(Lav_PROPERTYTYPE_FLOAT_ARRAY);
	prop->setArrayLengthRange(minLength, maxLength);
	std::vector<float> new_default;
	new_default.resize(defaultLength);
	std::copy(defaultData, defaultData+defaultLength, new_default.begin());
	prop->setFloatArrayDefault(new_default);
	prop->setName(name);
	prop->reset();	
	return prop;
}

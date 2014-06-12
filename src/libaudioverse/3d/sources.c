/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_all.h>
#include <transmat.h>
#include <stdlib.h>
#include <libaudioverse/private_sources.h>
#include <math.h>

LavPropertyTableEntry sourceProperties[] = {
	{Lav_SOURCE_POSITION, Lav_PROPERTYTYPE_FLOAT3, "position", {.f3val = {0.0f, 0.0f, 0.0f}}, NULL,},
	{Lav_SOURCE_MAX_DISTANCE, Lav_PROPERTYTYPE_FLOAT, "max_distance", {.fval = 50.0f}, NULL},
};

Lav_PUBLIC_FUNCTION LavError Lav_createMonoSource(LavObject* node, LavObject* world, LavObject** destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(node);
	CHECK_NOT_NULL(world);
	CHECK_NOT_NULL(destination);
	ERROR_IF_TRUE(node->num_outputs != 1, Lav_ERROR_SHAPE);
	//first, allocate and create as usual.
	LavObject* obj = calloc(1, sizeof(LavSource));
	ERROR_IF_TRUE(obj == NULL, Lav_ERROR_MEMORY);
	LavError err = initLavObject(0, 0,
	sizeof(sourceProperties)/sizeof(sourceProperties[0]), sourceProperties,
	Lav_OBJTYPE_SOURCE_MONO, world->device, obj);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	LavObject *atten, *panner;
	err = Lav_createHrtfNode(world->device, ((LavWorld*)world)->hrtf, &panner);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	err = Lav_createAttenuatorNode(world->device, 1, &atten);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//we connect the attenuator to the source node, because it's cheeper to attenuate before convolving/panning.
	err = Lav_setParent(atten, node, 0, 0);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//we then send the attenuator through the panner.
	err = Lav_setParent(panner, atten, 0, 0);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//assign these nodes to our struct.
	LavSource *source = (LavSource*)obj;
	source->data_node = node;
	source->panner_node = panner;
	source->attenuator_node = atten;
	worldAssociateSource((LavWorld*)world, source); //we're interested in playing, etc.
	*destination = obj; //use the object "view" of the source for this.
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

void sourceUpdateAzimuthElevation(LavSource* source) {
	//run our position in world coordinates through the camera transform, and then pull out elevation and azimuth.
	TmVector pos;
	Lav_getFloat3Property((LavObject*)source, Lav_SOURCE_POSITION, pos.vec, pos.vec+1, pos.vec+2);
	pos.vec[3] = 1.0f; //turn on translation.
	//now run through the camera matrix.
	Tm_transformApply(source->world->camera_transform, pos, &pos);
	//normalize pos.
	TmVector npos;
	float magnitude = pos.vec[0]*pos.vec[0]+pos.vec[1]*pos.vec[1]+pos.vec[2]*pos.vec[2];
	magnitude = sqrtf(magnitude);
	//if magnitude<1, we're "inside the listener"
	//todo: this needs to be configurable, and is just a quick way to prevent things from blowing up while I work on other stuff.
	if(magnitude < 1) {
		Lav_setFloatProperty(source->panner_node, Lav_HRTF_AZIMUTH, 0.0f);
		Lav_setFloatProperty(source->panner_node, Lav_HRTF_ELEVATION, 0.0f);
		return;
	}
	//if we get heree, we're far enough outside the listener's head that we can do the calculations.
	npos.vec[0] = pos.vec[0]/magnitude;
	npos.vec[1] = pos.vec[1]/magnitude;
	npos.vec[2] = pos.vec[2] / magnitude;
	//this is the standard cartesian to spherical conversion.
	const float x = pos.vec[0];
	const float y = pos.vec[1]; //the vertical component.
	const float z = pos.vec[2];
	const float xz = sqrtf(x*x+z*z); //the part in the xz plane.
	const float elevation = asinf(y/xz); //the elevation angle, from -pi to pi radians.
	//this next line took a while to get right.
	//basically, by negating the z axis and leaving everything else alone, angles start going clockwise.
	const float azimuth = atan2f(x, -z);
	//now we just set our panner.
	//recall that these want degrees.
	Lav_setFloatProperty(source->panner_node, Lav_HRTF_AZIMUTH, azimuth/PI*180.0f);
	Lav_setFloatProperty(source->panner_node, Lav_HRTF_ELEVATION, elevation/PI*180.0f);
	return;
}

void sourceUpdateAttenuation(LavSource* source) {
	float x1, y1, z1, x2, y2, z2;
	Lav_getFloat3Property((LavObject*)source->world, Lav_WORLD_LISTENER_POSITION, &x1, &y1, &z1);
	Lav_getFloat3Property((LavObject*)source, Lav_SOURCE_POSITION, &x2, &y2, &z2);
	const float dx=x2-x1, dy=y2-y1, dz=z2-z1;
	float magnitude = sqrtf(dx*dx+dy*dy+dz*dz);
	float max_distance;
	Lav_getFloatProperty((LavObject*)source, Lav_SOURCE_MAX_DISTANCE, &max_distance);
	magnitude = magnitude > max_distance? max_distance : magnitude;
	float atten = 1-magnitude/max_distance;
	Lav_setFloatProperty(source->attenuator_node, Lav_ATTENUATOR_MULTIPLIER, atten);
}

void sourceUpdate(LavSource* source) {
	sourceUpdateAzimuthElevation(source);
	sourceUpdateAttenuation(source);
}

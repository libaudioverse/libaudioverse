/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_all.h>
#include <transmat.h>
#include <stdlib.h>
#include <libaudioverse/private_sources.h>
#include <math.h>

LavPropertyTableEntry sourceProperties[] = {
	{Lav_SOURCE_POSITION, Lav_PROPERTYTYPE_FLOAT3, "position", {.f3val = {0, 0, 0}}, NULL,}
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
	Lav_createAttenuatorNode(world->device, 1, &atten);
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
	SAFERETURN(Lav_ERROR_NONE);
	*destination = obj; //use the object "view" of the source for this.
	STANDARD_CLEANUP_BLOCK;
}

void sourceUpdate(LavSource* source) {
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
	npos.vec[0] = pos.vec[0]/magnitude;
	npos.vec[1] = pos.vec[1]/magnitude;
	npos.vec[2] = pos.vec[2] / magnitude;
	//this is the standard cartesian to spherical conversion.
	const float x = pos.vec[0];
	const float y = pos.vec[1]; //the vertical component.
	const float z = pos.vec[2];
	const float xz = sqrtf(x*x+z*z); //the part in the xz plane.
	const float elevation = asinf(y/xz); //the elevation angle, from -pi to pi radians.
	//the weirdness is because we face -z, not -x, and we're taking angles relative to an x of -z.
	//by negating the x value, we get the angle to also fall clockwise, which is what panners need because of the hrtf data.
	const float azimuth = atan2f(-z, -x);
	//now we just set our panner.
	Lav_setFloatProperty(source->panner_node, Lav_HRTF_AZIMUTH, azimuth);
	Lav_setFloatProperty(source->panner_node, Lav_HRTF_ELEVATION, elevation);
	//todo: implement attenuation.
	return;
}

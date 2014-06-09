/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_all.h>
#include <libaudioverse/private_structs3d.h>
#include <libaudioverse/libaudioverse3d.h>
#include <transmat.h>
#include <stdlib.h>
#include <libaudioverse/private_sources.h>
LavPropertyTable sourceProperties[] = {
	{Lav_SOURCE_POSITION, Lav_PROPERTYTYPE_FLOAT3, "position", {.f3val = {0, 0, 0}}, NULL,}
};

Lav_PUBLIC_FUNCTION LavError Lav_createMonoSource(LavObject* node, LavObject* world, LavObject** destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(node);
	CHECK_NOT_NULL(world);
	CHECK_NOT_NULL(destination);
	ERROR_IF_TRUE(node->num_outputs != 1, Lav_ERROR_SHAPE);
	//first, allocate and create as usual.
	LavObject* ojb = calloc(1, sizeof(LavSource));
	ERROR_IF_TRUE(obj == NULL< Lav_ERROR_MEMORY);
	LavError err = initLavObject(0, 0,
	sizeof(sourceProperties)/sizeof(sourceProperties[0]), sourceProperties,
	Lav_OBJTYPE_SOURCE_MONO, world->mutex, &obj);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	lavObject *atten, *panner;
	err = Lav_createHrtfNode(((LavWorld*)world)->hrtf, ((LavWorld*)world)->graph, &panner);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	Lav_createAttenuatorNode(((LavWorld*)world)->graph, 1, &atten);
	//we connect the attenuator to the source node, because it's cheeper to attenuate before convolving/panning.
	err = Lav_setParent(attenuator, node, 0, 0);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//we then send the attenuator through the panner.
	err = Lav_setParent(panner, atten, 0, 0);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//assign these nodes to our struct.
	LavSource *source = (LavSource*)obj;
	source->data_node = node;
	source->panner_node = panner;
	source->attenuator_node = atten;
	worldAssociateSource(world, source); //we're interested in playing, etc.
	SAFERETURN(Lav_ERROR_NONE);
	*destination = obj; //use the object "view" of the source for this.
	STANDARD_CLEANUP_BLOCK;
}

void sourceUpdate(LavSource* source) {
}

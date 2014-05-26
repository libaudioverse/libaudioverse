/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>

LavError hrtfProcessor(LavNode *node, unsigned int count);

struct HrtfNodeData {
	int dummy; //for testing, just so we can compile.
};
typedef struct HrtfNodeData HrtfNodeData;

LavPropertyTableEntry hrtfPropertyTable[] = {
{Lav_HRTF_AZIMUTH, Lav_PROPERTYTYPE_FLOAT, "azimuth", {.fval = 0.0f}},
{Lav_HRTF_ELEVATION, Lav_PROPERTYTYPE_FLOAT, "elevation", {.fval = 0.0f}},
};

Lav_PUBLIC_FUNCTION Lav_createHrtfNode(LavGraph *graph, LavHrtfData* hrtf, LavNode **destination) {
	WILL_RETURN(LavError);
	LavError err = Lav_ERROR_NONE;
	CHECK_NOT_NULL(hrtf);
	CHECK_NOT_NULL(destination);
	LOCK(graph->mutex);
	LavNode* retval = NULL;
		err = Lav_createNode(1, 2, Lav_NODETYPE_HRTF, graph, &retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);

	retval->properties = makePropertyArrayFromTable(sizeof(hrtfPropertyTable)/sizeof(hrtfPropertyTable[0]), hrtfPropertyTable);
	ERROR_IF_TRUE(retval->properties, Lav_ERROR_MEMORY);
	RETURN(Lav_ERROR_NONE);
	retval->process = hrtfProcessor;

	HrtfNodeData *data = calloc(1, sizeof(HrtfNodeData));
	ERROR_IF_TRUE(data == NULL, Lav_ERROR_MEMORY);

	*destination = retval;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(graph->mutex);
}

LavError hrtfProcessor(LavNode *node, unsigned int count) {
	return Lav_ERROR_NONE;
}

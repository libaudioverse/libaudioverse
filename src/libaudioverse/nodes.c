/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Specifically inits and deinits nodes.*/

#include <stdlib.h>
#include <string.h>
#include <libaudioverse/private_all.h>


Lav_PUBLIC_FUNCTION LavError Lav_createNode(unsigned int numInputs, unsigned int numOutputs, unsigned int numProperties, LavPropertyTableEntry* propertyTable, enum Lav_NODETYPES type, LavObject* graph, LavObject** destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(destination);
	LavNode* retval = calloc(1, sizeof(LavNode));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	LavError err = initLavObject(numInputs, numOutputs, numProperties, propertyTable, type, graph->block_size, graph->mutex, (LavObject*)retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	graphAssociateNode(graph, (LavObject*)retval);
	retval->graph = (LavGraph*)graph;
	*destination = (LavObject*)retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}
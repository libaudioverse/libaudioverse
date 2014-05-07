#include <libaudioverse/private_all.h>
#include <stdlib.h>

Lav_PUBLIC_FUNCTION LavError Lav_createGraph(LavGraph **destination) {
	LavGraph *retval = calloc(1, sizeof(LavGraph));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	*destination = retval;
	return Lav_ERROR_NONE;
}

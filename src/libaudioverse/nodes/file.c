/**Reads an entire file with Libsndfile.  At the moment, this file must be specified at construction time.*/
#include <libaudioverse/private_all.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>

struct fileinfo {
	SF_INFO info;
	float table_delta;
	LavTable **tables;
	float current_index;
};

Lav_PUBLIC_FUNCTION Lav_createFileNode(LavGraph *graph, const char* path, LavNode** destination) {
}

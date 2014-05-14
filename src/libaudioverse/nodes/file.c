/**Reads an entire file with Libsndfile.  At the moment, this file must be specified at construction time.*/
#include <libaudioverse/private_all.h>

struct fileinfo {
	SNDFILE *handle;
	SF_INFO info;
	float table_delta;
	LavTable *table;
};

Lav_PUBLIC_FUNCTION Lav_createFileNode(LavGraph *graph, const char* path, LavNode** destination) {

}

Lav_PUBLIC_FUNCTION LavError fileProcessor(LavNode* node, unsigned int count) {
	return lav_ERROR_NONE;
}

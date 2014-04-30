/**The public interface to Libaudioverse.*/

/**These are property types, either int, float, double, or string.

Note that they can be ored.  This is important and intended as an extension point.  Namely, some properties in future are going to allow themselves to be either a constant or an LFO.*/
enum Lav_PropertyTypes {
	Lav_PROPERTYTYPE_INT = 0x1,
	Lav_PROPERTYTYPE_FLOAT = 0x2,
	Lav_PROPERTYTYPE_DOUBLE = 0x4,
	Lav_PROPERTYTYPE_STRING = 0x8,
};

/**Whenh is this property processed?  Either block or per-sample.*/
enum lav_PropertyResolution {
	Lav_PROPERTYRESOLUTION_BLOCK = 0x1,
	Lav_PROPERTYRESOLUTION_SAMPLE = 0x2,
};

struct Lav_Property_s {
	enum LavPropertyType type;
	enum LavPropertyResolution resolution;
};

typedef struct Lav_Property_s LavProperty;

struct Lav_Buffer_s {
	int foo;
};

typedef struct Lav_Buffer_s LavBuffer;

struct Lav_Node_s {
	int foo;\
};

typedef struct Lav_Node_s LavNode;
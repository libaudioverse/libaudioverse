/**The public interface to xxx.*/

/**These are property types, either int, float, double, or string.
Note that they can be ored.  This is important and intended as an extension point.  Namely,
some properties in future are going to allow themselves to be either a constant or an LFO.*/
enum PropertyTypes {
PROPERTYTYPE_INT = 0x1,
PROPERTYTYPE_FLOAT = 0x2,
PROPERTYTYPE_STRING = 0x4,
};

/**Whenh is this property processed?  Either block or per-sample.*/
enum PropertyResolution {
PROPERTYRESOLUTION_BLOCK = 0x1,
PROPERTYRESOLUTION_SAMPLE = 0x2,
};

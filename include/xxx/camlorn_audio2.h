/**The public interface to camlorn_audio2.*/

/**These are property types, either int or float.*/
#define CA_PROPERTY_TYPE_INT 0
#define CA_PROPERTY_TYPE_FLOAT 1

/**Whenh is this property processed?  Either block or per-sample.*/
#define CA_PROPERTY_RESOLUTION_BLOCK 0
#define CA_PROPERTY_RESOLUTION_SAMPLE 1

/**Represents a property slot.  This is used to allow for some internal type checking, as some properties are dynamic, etc.
Properties may not be overridden by client code, merely plugged in.  Therefore, methods are hard-coded.*/
struct CA_Property {
	unsigned int type, resolution;
	union {
		int ival;
		float fval;
	} value;
};

struct CA_Node;
/**Represents pointers to other nodes.
Note that this also should not be overridden by client code.
This exists as an extension point for the future: cycles will require more properties here.*/
struct CA_Edge {
	struct CA_Node *endpoint;
};

/**The basic object which represnts all nodes in the signal graph.*/
struct CA_Node;

typedef int (*NODE_PROCESS_PTR)(unsigned int samples_to_process); //parameter is number of samples to compute.
typedef int (*NODE_SET_PARENT_PTR)(struct CA_Node *new_parent, unsigned int slot); //Set a node's parent.

struct CA_Node {
	struct CA_Property *properties;
	unsigned int num_properties;
	struct CA_Edge *inputs;
	unsigned int num_parents;
	float *input_buffer, *output_buffer; //places for computed samples to go.
	float *history_buffer; //Places for nodes which need history to save it.
	//Functions.
	NODE_PROCESS_PTR process; //The function which can give us more samples.
	NODE_SET_PARENT_PTR set_parent; //Set a parent.
};

/**Private macro definitions.*/

#define ERROR_IF_TRUE(expression, error) do {\
if(expression) return error;\
} while(0)

#define CHECK_NOT_NULL(ptr) ERROR_IF_TRUE(ptr == NULL, Lav_ERROR_NULL_POINTER)

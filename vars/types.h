#ifndef VAR_TYPES_H
#define VAR_TYPES_H

#include <stdbool.h>
#include "../arrays.h"
#include "../types.h"

// name is an out parameter
c_type *get_var_type(rarray *raw_elems, bool is_function, char **name);

#endif

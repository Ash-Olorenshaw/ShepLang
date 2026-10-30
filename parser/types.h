
#ifndef __PARSER_VARS_H
#define __PARSER_VARS_H

#include <stdbool.h>
#include "../utils/arrays.h"
#include "../types.h"
#include "../tokeniser.h"

c_type *get_tkns_type(rarray *tkns, int depth);
bool compare_function_args(rarray *fn_args, tkn *container_tkn);

#endif

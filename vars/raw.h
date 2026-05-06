#ifndef VAR_RAW_H
#define VAR_RAW_H

#include "../arrays.h"

typedef struct {
	char *elem;
	int start;
} var_elem;

rarray *get_var_elems(const char *line);

#endif

#ifndef __PARSER_UTILS_H
#define __PARSER_UTILS_H

#include <stdbool.h>
#include "../utils/arrays.h"
#include "../vars/types.h"

typedef struct {
	enum {
		IDENTIFIER_UNKNOWN,
		IDENTIFIER_STACK_ALLOCATED,
		IDENTIFIER_HEAP_ALLOCATED
	} location;
	enum {
		IDENTIFIER_UNKNOWN_MUTABILITY,
		IDENTIFIER_MUTABLE,
		IDENTIFIER_IMMUTABLE
	} mutable;
	enum {
		IDENTIFIER_UNKNOWN_OWNERSHIP,
		IDENTIFIER_IN,
		IDENTIFIER_OUT,
		IDENTIFIER_INOUT,
	} ownership;
	char *name;
	bool freed;
	c_type *type;
} identifier;

extern rarray *identifier_scopes; // identifier

void init_identifier_scopes(void);
rarray *create_new_scope(void);
identifier *scope_find(char *target);
void add_to_scope(identifier *item);
void exit_scope(void);

#endif

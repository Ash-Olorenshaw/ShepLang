#ifndef __PARSER_UTILS_H
#define __PARSER_UTILS_H

#include <stdbool.h>
#include "../utils/arrays.h"

typedef enum { 
	IDENTIFIER_VAR,
	IDENTIFIER_FUNC 
} identifier_type;

typedef struct {
	identifier_type type;
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
	char *name;
} identifier;

extern rarray *identifier_scopes; // identifier

void init_identifier_scopes(void);
rarray *create_new_scope(void);
identifier *scope_find(char *target, identifier_type type);
void add_to_scope(identifier *item);
void exit_scope(void);

#endif

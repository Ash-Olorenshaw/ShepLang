#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "./vars.h"
#include "../utils/arrays.h"

rarray *identifier_scopes;
rarray *current_scope;

void init_identifier_scopes(void) {
	identifier_scopes = rarray_create(10, sizeof(rarray*));
	create_new_scope();
}

rarray *create_new_scope(void) {
	current_scope = rarray_create(10, sizeof(identifier*));
	rarray_add(identifier_scopes, current_scope);
	return current_scope;
}

identifier *scope_find(char *target, identifier_type type) {
	rarray *scope;
	int i;
	RARRAY_FOREACH(scope, identifier_scopes, i) {
		identifier *elem;
		int j;
		RARRAY_FOREACH(elem, scope, j) {
			if (elem->type == type && strcmp(target, elem->name) == 0)
				return elem;
		}
	}
	return NULL;
}

void add_to_scope(identifier *item) {
	if (scope_find(item->name, item->type))
		printf("WARN - %s \"%s\" is already defined, this is a shadow definition.\n", item->type == IDENTIFIER_VAR ? "variable" : "function", item->name);
	rarray_add(current_scope, item);
}

void exit_scope(void) {
	rarray *popped = rarray_pop(identifier_scopes);
	current_scope = identifier_scopes->items[identifier_scopes->size - 1];
	// TODO - fix cleanup
	// if (popped != NULL)
	// 	rarray_free(popped);
}


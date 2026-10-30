#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./vars.h"
#include "../utils/core.h"
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

identifier *scope_find(char *target) {
	rarray *scope;
	int i;
	RARRAY_FOREACH(scope, identifier_scopes, i) {
		identifier *elem;
		int j;
		RARRAY_FOREACH(elem, scope, j) {
			if (strcmp(target, elem->name) == 0)
				return elem;
		}
	}
	return NULL;
}

void add_to_scope(identifier *item) {
	if (scope_find(item->name))
		printf("WARN - \"%s\" is already defined, this is a shadow definition.\n", item->name);
	rarray_add(current_scope, item);
}

void exit_scope(void) {
	rarray **popped_ptr = (rarray **) rarray_pop(identifier_scopes);
	rarray *popped = *popped_ptr;
	if (popped != NULL) {
		identifier *elem = NULL;
		int i;
		RARRAY_FOREACH(elem, popped, i) {
			if (elem->type->type != C_PTR) {
				bool is_in = false;
				RARRAY_CONTAINS(elem->type->ptr.modifiers, IN, c_type_ptr_modifier, is_in);
				if (is_in && !elem->freed)
					raise_err("Var %s is not freed", elem->name);
			}

			free(elem->name);
		}
		rarray_free(popped);
	}
	if (identifier_scopes->size > 0)
		current_scope = identifier_scopes->items[identifier_scopes->size - 1];
	else current_scope = NULL;
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../utils/strings/utils.h"
#include "../utils/arrays.h"
#include "../utils/core.h"
#include "../types.h"
#include "./compound_types.h"
#include "./raw.h"


void assign_var_type_and_mod(rarray *elems, int elems_start, c_type **next_item, c_type **first_ptr_item) {
	// helper func that basically consumes all "mods" for a var type and then consumes the type
	char *elem0 = ((var_elem *)elems->items[elems_start])->elem;
	printf("Is type? %s : %s\n", elem0, is_type(elem0) ? "true" : "false");
	if (is_type(elem0) || is_ptr_type_mod(elem0) || is_simple_type_mod(elem0)) {
		int pos = elems_start + 1;
		char *nxt_elem = elem0;
		rarray *simple_mods = rarray_create(2, sizeof(c_type_simple_modifier));
		rarray *ptr_mods = rarray_create(5, sizeof(c_type_ptr_modifier));
		while (is_ptr_type_mod(nxt_elem) || is_simple_type_mod(nxt_elem)) {
			printf("Adding mod: %s\n", nxt_elem);
			if (is_simple_type_mod(nxt_elem)) {
				c_type_simple_modifier *mod = malloc(sizeof(c_type_simple_modifier));
				*mod = get_simple_type_mod(nxt_elem);
				rarray_add(simple_mods, mod);
			}
			else if (first_ptr_item != NULL) {
				c_type_ptr_modifier *mod = malloc(sizeof(c_type_ptr_modifier));
				*mod = get_ptr_type_mod(nxt_elem);
				rarray_add(ptr_mods, mod);
			}
			nxt_elem = ((var_elem *)elems->items[pos++])->elem;
			if (pos == elems->size) raise_err("Incomplete var definition.");
		}
		*next_item = get_type(nxt_elem);
		printf("Generated next_item: %s (%d)\n", nxt_elem, (*next_item)->type);
		(*next_item)->simple.modifiers = simple_mods;
		if (first_ptr_item != NULL)
			(*first_ptr_item)->ptr.modifiers = ptr_mods;
		else
			rarray_free(ptr_mods);
	}
	else {
		(*next_item)->simple.modifiers = NULL;
		(*first_ptr_item)->ptr.modifiers = NULL;
	}
	printf("Assigned type and mods\n");
	print_type(*next_item, true);
}

// name must be an unitialised string
c_type *get_var_type(rarray *raw_elems, bool is_function, char **name) {
	// TODO - need to free "raw_elems"
	c_type *result = malloc(sizeof(c_type));
	c_type **next_item;
	c_type **first_ptr = NULL;
	int i;

	is_function = is_function && strcmp(((var_elem *)raw_elems->items[0])->elem, "struct") != 0;

	if (is_function) {
		result->type = C_FN;
		result->fn.type = malloc(sizeof(struct c_type));
		next_item = &(result->fn.type);
	}
	else
		next_item = &result;

	for (i = 0; i < raw_elems->size; i++) {
		const char* elem = ((var_elem *)raw_elems->items[i])->elem;
		printf("RAW ELEM: %s\n", elem);
		if (strcmp(elem, "*") == 0) {
			(*next_item)->type = C_PTR;
			(*next_item)->ptr.to = malloc(sizeof(struct c_type));
			if (first_ptr == NULL) first_ptr = next_item;
			next_item = &((*next_item)->ptr.to);
		}
		else if (strcmp(elem, "&") == 0) {
			(*next_item)->type = C_ADR;
			(*next_item)->adr.to = malloc(sizeof(struct c_type));
			next_item = &((*next_item)->adr.to);
		}
		else if (elem[0] == '[' && elem[strlen(elem) - 1] == ']') {
			(*next_item)->type = C_ARR;
			(*next_item)->arr.size_identifier = NULL;
			if (strlen(elem) > 2) {
				char *num_str = substr(elem, 1, strlen(elem) - 1);
				bool is_digit = true;
				for (int i = 0; i < (int) strlen(num_str); i++)
					if (!isdigit(num_str[i])) {
						is_digit = false;
						break;
					}
				if (is_digit)
					(*next_item)->arr.size_int = atoi(num_str);
				else
					(*next_item)->arr.size_identifier = strdup(num_str);
				free(num_str);
			}
			else (*next_item)->arr.size_int = -1;

			(*next_item)->arr.of = malloc(sizeof(struct c_type));
			next_item = &(*next_item)->arr.of;
		}
		else if (!is_simple_type_mod(elem) && !is_ptr_type_mod(elem) && !is_type(elem) && !is_cmplx_type(elem) && str_alnum(elem) && *name == NULL) {
			*name = strdup(elem);
			if (is_function)
				break;
		}
	}

	assign_var_type_and_mod(raw_elems, 0, next_item, first_ptr);

	if (strcmp(((var_elem*)raw_elems->items[0])->elem, "struct") == 0) {
		(*next_item)->type = C_STRT;
		(*next_item)->strt.members = rarray_create(20, sizeof(strt_member*));
		char* struct_body;

		if (((var_elem*)raw_elems->items[1])->elem[0] == '{') {
			(*next_item)->strt.name = NULL;
			struct_body = strdup(((var_elem *)raw_elems->items[1])->elem);
		}
		else {
			result->strt.name = strdup(((var_elem *)raw_elems->items[1])->elem);
			struct_body = strdup(((var_elem *)raw_elems->items[2])->elem);
		}

		process_compound_type(struct_body, next_item, C_STRT);
		if (raw_elems->size <= 2) {
			result->strt.name = strdup(((var_elem *)raw_elems->items[1])->elem);
			return result;
		}
	}
	else if (strcmp(((var_elem *)raw_elems->items[0])->elem, "union") == 0) {
		(*next_item)->type = C_UNN;
		(*next_item)->strt.members = rarray_create(20, sizeof(strt_member*));
		char* struct_body;

		if (((var_elem*)raw_elems->items[1])->elem[0] == '{') {
			(*next_item)->strt.name = NULL;
			struct_body = strdup(((var_elem *)raw_elems->items[1])->elem);
		}
		else {
			result->strt.name = strdup(((var_elem *)raw_elems->items[1])->elem);
			struct_body = strdup(((var_elem *)raw_elems->items[2])->elem);
		}

		process_compound_type(struct_body, next_item, C_UNN);
		if (raw_elems->size <= 2) {
			result->strt.name = strdup(((var_elem *)raw_elems->items[1])->elem);
			return result;
		}
	}
	else if (is_function) {
		printf("Hey this is a function\n");
		fn_arg *current_arg = malloc(sizeof(fn_arg));
		c_type *parent_item = malloc(sizeof(struct c_type));
		c_type **next_item = &(parent_item);
		c_type **first_ptr = NULL;

		int start = i + 1, arg_count = 0;
		bool skip = false;

		result->fn.args = rarray_create(10, sizeof(fn_arg*));

		for (i = start + 1; i < raw_elems->size; i++) {
			if (skip) {
				skip = false;
				continue;
			}

			char* elem = ((var_elem *)raw_elems->items[i])->elem;

			if (strcmp(elem, "*") == 0) {
				(*next_item)->type = C_PTR;
				(*next_item)->ptr.to = malloc(sizeof(struct c_type));
				if (first_ptr == NULL)
					first_ptr = next_item;
				next_item = &((*next_item)->ptr.to);
			}
			else if (strcmp(elem, "&") == 0) {
				(*next_item)->type = C_ADR;
				(*next_item)->adr.to = malloc(sizeof(struct c_type));
				next_item = &((*next_item)->adr.to);
			}
			else if (elem[0] == '[' && elem[strlen(elem) - 1] == ']') {
				(*next_item)->type = C_ARR;
				(*next_item)->arr.size_identifier = NULL;
				if (strlen(elem) > 2) {
					char *num_str = substr(elem, 1, strlen(elem) - 1);
					bool is_digit = true;
					for (int i = 0; i < (int) strlen(num_str); i++)
						if (!isdigit(num_str[i])) {
							is_digit = false;
							break;
						}
					if (is_digit)
						(*next_item)->arr.size_int = atoi(num_str);
					else
						raise_err("You cannot use identifiers to describe array length in function prototypes: '%s'", elem);
					free(num_str);
				}
				else (*next_item)->arr.size_int = -1;

				(*next_item)->arr.of = malloc(sizeof(struct c_type));
				next_item = &((*next_item)->arr.of);
			}
			else if (elem[strlen(elem) - 1] == ',' || i == raw_elems->size - 1) {
				if (strcmp(elem, ",") == 0) {
					if (i - 2 > 0)
						elem = ((var_elem *)raw_elems->items[i - 2])->elem;
					else
						raise_err("Unable to identify name of var '%s'.", elem);
				}
				assign_var_type_and_mod(raw_elems, start, next_item, first_ptr);
				if (i == raw_elems->size - 1)
					current_arg->name = elem;
				else {
					char *new_elem;
					if (elem[strlen(elem) - 1] == ',')
						new_elem = substr(elem, 0, strlen(elem) - 1);
					else
						new_elem = strdup(elem);

					current_arg->name = new_elem;
				}
				current_arg->type = parent_item;
				rarray_add(result->fn.args, current_arg);
				arg_count++;
				if (i != raw_elems->size - 1) {
					if (i + 1 >= raw_elems->size)
						raise_err("Trailing comma.");
					current_arg = malloc(sizeof(fn_arg));
					parent_item = malloc(sizeof(struct c_type));
					next_item = &parent_item;
				}
				start = i + 1;
				skip = true;
			}
		}

		result->fn.arg_count = arg_count;
	}

	rarray_free(raw_elems);
	printf("Created type: %d (vs: %d)\n", result->type, (*next_item)->type);
	return result;
}

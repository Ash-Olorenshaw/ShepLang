#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../string_utils.h"
#include "../arrays.h"
#include "../utils.h"
#include "../types.h"
#include "./compound_types.h"
#include "./raw.h"

// name must be an unitialised string
c_type *get_var_type(rarray *raw_elems, bool is_function, char **name) {
	c_type *result = malloc(sizeof(c_type));
	c_type *next_item;
	int i;

	is_function = is_function && strcmp(((var_elem *)raw_elems->items[0])->elem, "struct") != 0;

	if (is_function) {
		result->type = C_FN;
		result->fn.type = malloc(sizeof(struct c_type));
		next_item = result->fn.type;
	}
	else
		next_item = result;

	for (i = 0; i < raw_elems->size; i++) {
		const char* elem = ((var_elem *)raw_elems->items[i])->elem;
		if (strcmp(elem, "*") == 0) {
			next_item->type = C_PTR;
			next_item->ptr.to = malloc(sizeof(struct c_type));
			next_item = next_item->ptr.to;
		}
		else if (strcmp(elem, "&") == 0) {
			next_item->type = C_ADR;
			next_item->adr.to = malloc(sizeof(struct c_type));
			next_item = next_item->adr.to;
		}
		else if (elem[0] == '[' && elem[strlen(elem) - 1] == ']') {
			next_item->type = C_ARR;
			if (strlen(elem) > 2) {
				char *num_str = substr(elem, 1, strlen(elem) - 1);
				next_item->arr.size = atoi(num_str);
				free(num_str);
			}
			else next_item->arr.size = -1;

			next_item->arr.of = malloc(sizeof(struct c_type));
			next_item = next_item->arr.of;
		}
		else if (!is_type(elem) && strcmp(elem, "struct") != 0 && strcmp(elem, "enum") != 0 && strcmp(elem, "union") != 0 && str_alnum(elem) && *name == NULL) {
			*name = strdup(elem);
			if (is_function)
				break;
		}
	}

	printf("GENERATING TYPE: %s\n", ((var_elem *)raw_elems->items[0])->elem);
	if (is_type(((var_elem *)raw_elems->items[0])->elem))
		*next_item = get_type(((var_elem *)raw_elems->items[0])->elem);

	if (strcmp(((var_elem*)raw_elems->items[0])->elem, "struct") == 0) {
		next_item->type = C_STRT;
		next_item->strt.members = rarray_create(20, sizeof(strt_member*));
		char* struct_body;

		if (((var_elem*)raw_elems->items[1])->elem[0] == '{') {
			next_item->strt.name = NULL;
			struct_body = ((var_elem *)raw_elems->items[1])->elem;
		}
		else {
			result->strt.name = ((var_elem *)raw_elems->items[1])->elem;
			struct_body = ((var_elem *)raw_elems->items[2])->elem;
		}

		process_compound_type(struct_body, &next_item, C_STRT);
		if (raw_elems->size <= 2) {
			result->strt.name = ((var_elem *)raw_elems->items[1])->elem;
			return result;
		}
	}
	else if (strcmp(((var_elem *)raw_elems->items[0])->elem, "union") == 0) {
		next_item->type = C_UNN;
		next_item->strt.members = rarray_create(20, sizeof(strt_member*));
		char* struct_body;

		if (((var_elem*)raw_elems->items[1])->elem[0] == '{') {
			next_item->strt.name = NULL;
			struct_body = ((var_elem *)raw_elems->items[1])->elem;
		}
		else {
			result->strt.name = ((var_elem *)raw_elems->items[1])->elem;
			struct_body = ((var_elem *)raw_elems->items[2])->elem;
		}

		process_compound_type(struct_body, &next_item, C_UNN);
		if (raw_elems->size <= 2) {
			result->strt.name = ((var_elem *)raw_elems->items[1])->elem;
			return result;
		}
	}
	else if (is_function) {
		fn_arg *current_arg = malloc(sizeof(fn_arg));
		struct c_type *parent_item = malloc(sizeof(struct c_type));
		struct c_type *next_item = parent_item;

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
				next_item->type = C_PTR;
				next_item->ptr.to = malloc(sizeof(struct c_type));
				next_item = next_item->ptr.to;
			}
			else if (strcmp(elem, "&") == 0) {
				next_item->type = C_ADR;
				next_item->adr.to = malloc(sizeof(struct c_type));
				next_item = next_item->adr.to;
			}
			else if (elem[0] == '[' && elem[strlen(elem) - 1] == ']') {
				next_item->type = C_ARR;
				if (strlen(elem) > 2) {
					char *num_str = substr(elem, 1, strlen(elem) - 1);
					next_item->arr.size = atoi(num_str);
					free(num_str);
				}
				else next_item->arr.size = -1;

				next_item->arr.of = malloc(sizeof(struct c_type));
				next_item = next_item->arr.of;
			}
			else if (elem[strlen(elem) - 1] == ',' || i == raw_elems->size - 1) {
				*next_item = get_type(((var_elem *)raw_elems->items[start])->elem);
				if (i == raw_elems->size - 1)
					current_arg->name = elem;
				else {
					char *new_elem = substr(elem, 0, strlen(elem) - 1);
					current_arg->name = new_elem;
					free(elem);
				}
				current_arg->type = parent_item;
				rarray_add(result->fn.args, current_arg);
				arg_count++;
				if (i != raw_elems->size - 1) {
					if (i + 1 >= raw_elems->size)
						raise_err("Trailing comma.");
					current_arg = malloc(sizeof(fn_arg));
					parent_item = malloc(sizeof(struct c_type));
					next_item = parent_item;
				}
				start = i + 1;
				skip = true;
			}
		}

		result->fn.arg_count = arg_count;
	}

	return result;
}

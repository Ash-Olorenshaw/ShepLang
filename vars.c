#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./string_utils.h"
#include "./utils.h"
#include "./vars/types.h"
#include "./vars/raw.h"

var_elem *find_var_name(rarray *raw_elems) {
	var_elem *elem;
	int i;
	RARRAY_FOREACH(elem, raw_elems, i) {
		if (strcmp(elem->elem, "*") != 0 && strcmp(elem->elem, "&") != 0 && !(elem->elem[0] == '[' && elem->elem[strlen(elem->elem) - 1] == ']') && !is_type(elem->elem))
			return elem;
	}
	return NULL;
}

c_type *get_var_info(const char *line) {
	char *line_copy;
	line_copy = strdup(line);
	remove_unnecessary_whitespace(line_copy);
	printf("Getting var info: %s\n", line);
	rarray *raw_elems = get_var_elems(strdup(line)); // array of var_elem

	// for (int i = 0; i < raw_elems->size; i++)
	// 	printf("%s\n", ((var_elem *)raw_elems->items[i])->elem);

	// in a few cases we will have the [] be the last item, so this just swaps it and the var/func name
	if (((var_elem *)raw_elems->items[raw_elems->size - 1])->elem[0] == '[' && raw_elems->size > 1)
		swap_ptrs(&raw_elems->items[raw_elems->size - 1], &raw_elems->items[raw_elems->size - 2]);

	var_elem *var_name_obj = find_var_name(raw_elems);
	bool is_function = str_find(line_copy, ")(") != -1 || (var_name_obj != NULL && line[var_name_obj->start + strlen(var_name_obj->elem)] == '(');

	char *var_name = NULL;
	c_type *result = get_var_type(raw_elems, is_function, &var_name);
	printf("VARNAME: %s\n", var_name);
	print_type(*result, true);

	rarray_free(raw_elems); // TODO - this isn't a proper free since each thing is now a struct...
	free(line_copy);

	printf("\n");
	return result;
}

#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "../core.h"

string_builder *string_builder_create(int size) {
	string_builder *new_string_builder = malloc(sizeof(string_builder));

	if (new_string_builder == NULL)
		return NULL;

	new_string_builder->string = malloc(size * sizeof(char));

	if (new_string_builder->string == NULL)
		return NULL;

	new_string_builder->max_size = size;
	new_string_builder->size = 0;
	new_string_builder->string[0] = '\0';
	return new_string_builder;
}

// positive == success
int string_builder_reset(string_builder *target) {
	target->size = 0;
	target->string[0] = '\0';
	return 1;
}

// positive == success
int string_builder_soft_reset(string_builder *target) {
	target->size = 0;
	return 1;
}

// positive == success
int string_builder_free(string_builder **target) {
	free((*target)->string);
	free(*target);
	return 1;
}

// positive == success
int string_builder_resize(string_builder *target) {
	void *new_items = realloc(target->string, target->max_size * 2 * sizeof(char));
	if (new_items == NULL)
		return 0;

	target->string = new_items;
	target->max_size = target->max_size * 2;
	return 1;
}

// positive == success
int string_builder_add_c(string_builder *target, char c) {
	if (target->size + 1 >= target->max_size && !string_builder_resize(target))
		return 0;
	else if (target == NULL)
		raise_err("Failed to add null char to string_builder");

	target->string[target->size++] = c;
	target->string[target->size] = '\0';
	return 1;
}

int string_builder_raw_add_c(string_builder *target, char c) {
	if (target->size + 1 >= target->max_size && !string_builder_resize(target))
		return 0;
	else if (target == NULL)
		raise_err("Failed to add null char to string_builder");

	target->string[target->size++] = c;
	return 1;
}

int string_builder_add_s(string_builder *target, char *s) {
	if (target->size + ((int) strlen(s)) >= target->max_size && !string_builder_resize(target))
		return 0;
	else if (target == NULL || s == NULL)
		raise_err("Failed to add null char to string_builder");

	for (int i = 0; i < (int) strlen(s); i++)
		if (!string_builder_raw_add_c(target, s[i]))
			raise_err("Failed to add string to string_builder");
	target->string[target->size] = '\0';
	return 1;
}

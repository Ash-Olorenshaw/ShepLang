#ifndef __UTILS_STRINGS_TYPE_H
#define __UTILS_STRINGS_TYPE_H

typedef struct {
	char *string;
	int size, max_size;
} string_builder;

string_builder *string_builder_create(int size);
int string_builder_reset(string_builder *target);
int string_builder_soft_reset(string_builder *target);
int string_builder_free(string_builder **target);
int string_builder_resize(string_builder *target);
int string_builder_add_c(string_builder *target, char c);
int string_builder_raw_add_c(string_builder *target, char c);
int string_builder_add_s(string_builder *target, char *s);

#endif

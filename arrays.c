#include <stdio.h>
#include <stdlib.h>
#include "arrays.h"
#include "utils.h"

rarray *rarray_create(int size, int item_size) {
	rarray *new_array = malloc(sizeof(rarray));

	if (new_array == NULL)
		return NULL;

	new_array->items = malloc(size * item_size);

	if (new_array->items == NULL)
		return NULL;

	new_array->max_size = size;
	new_array->size = 0;
	new_array->item_size = item_size;
	return new_array;
}

// positive == success
int rarray_free(rarray *array) {
	for (int i = 0; i < array->size; i++)
		free(array->items[i]);
	free(array->items);
	free(array);
	return 1;
}

// positive == success
int rarray_resize(rarray *array) {
	// printf(">>> Resizing array of size %d to size %d\n", array->max_size * array->item_size, array->max_size * array->item_size * 2);
	void **new_items = realloc(array->items, array->max_size * array->item_size * 2);
	if (new_items == NULL)
		return 0;

	array->items = new_items;
	array->max_size = array->max_size * 2;
	return 1;
}

// positive == success
int rarray_add(rarray *array, void *item) {
	if (array->size >= array->max_size && !rarray_resize(array))
		return 0;
	else if (item == NULL)
		raise_err("Failed to add null element to array");

	array->items[array->size++] = item;
	return 1;
}


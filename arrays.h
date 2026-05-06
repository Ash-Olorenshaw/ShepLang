#ifndef ARRAYS_H
#define ARRAYS_H

typedef struct {
	int max_size, item_size, size;
	void **items;
} rarray;

// requires including 'utils.h' too
#define RARRAY_ASSIGN(array, items) { \
	int i = 0; \
	while (items[i] != NULL) { \
		if (!rarray_add(array, items[i++])) \
			raise_err("Failed to add item to array."); \
	} \
}

#define RARRAY_FOREACH(elem, target, i) \
    for (i = 0; (target) != NULL && i < (target)->size && ((elem) = (target)->items[i]) != NULL; i++)

rarray *rarray_create(int size, int item_size);
int rarray_free(rarray *array);
int rarray_resize(rarray *array);
int rarray_add(rarray *array, void *item);

#endif

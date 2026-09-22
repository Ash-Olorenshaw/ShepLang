#include <stdlib.h>

void swap_ptrs(void **a, void **b) {
	void *c = *a;
	*a = *b;
	*b = c;
}


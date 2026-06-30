#include <stdio.h>
#include <stdlib.h>

void raise_err(const char *msg) {
	fprintf(stderr, "ERROR: %s\n", msg);
	exit(1);
}

void swap_ptrs(void **a, void **b) {
	void *c = *a;
	*a = *b;
	*b = c;
}


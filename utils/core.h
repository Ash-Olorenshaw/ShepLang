#ifndef __UTILS_CORE_H
#define __UTILS_CORE_H

#include <stdio.h>
#include <stdlib.h>

#define raise_err(...) { \
	printf("ERROR: "); \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	exit(1); \
}

void swap_ptrs(void **a, void **b);

#endif

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "types.h"
#include "utils.h"

bool is_type(const char *str) {
	return
		strcmp(str, "int") == 0 ||
		strcmp(str, "float") == 0 ||
		strcmp(str, "double") == 0 ||
		strcmp(str, "char") == 0 ||
		strcmp(str, "bool") == 0;
}

c_type get_type(const char *str) {
	c_type result = {0};
	if (strcmp(str, "int") == 0) {
		result.type = C_SIMPLE;
		result.simple.type = INT;
	}
	else if (strcmp(str, "float") == 0) {
		result.type = C_SIMPLE;
		result.simple.type = FLOAT;
	}
	else if (strcmp(str, "double") == 0) {
		result.type = C_SIMPLE;
		result.simple.type = DOUBLE;
	}
	else if (strcmp(str, "char") == 0) {
		result.type = C_SIMPLE;
		result.simple.type = CHAR;
	}
	else if (strcmp(str, "bool") == 0) {
		result.type = C_SIMPLE;
		result.simple.type = BOOL;
	}
	else {
		raise_err("Failed to parse type that does not exist (not 'int', 'float', 'double', 'char' or 'bool')");
	}

	return result;
}

void free_type(c_type *target) {
	rarray *mem = rarray_create(10, sizeof(c_type *));
	c_type *next_item = target;
	while (true) {
		switch (next_item->type) {
			case C_PTR:
				rarray_add(mem, next_item);
				next_item = next_item->ptr.to;
				break;
			case C_ADR:
				rarray_add(mem, next_item);
				next_item = next_item->adr.to;
				break;
			case C_ARR:
				rarray_add(mem, next_item);
				next_item = next_item->arr.of;
				break;
			// TODO enums, etc
		}
	}
}

void print_type(c_type target, bool newline) {
	if (target.type == C_SIMPLE) {
		printf("C_SIMPLE: ");
		if (target.simple.type == INT)
			printf("INT ");
		else if (target.simple.type == FLOAT)
			printf("FLOAT ");
		else if (target.simple.type == DOUBLE)
			printf("DOUBLE ");
		else if (target.simple.type == CHAR)
			printf("CHAR ");
		else if (target.simple.type == BOOL)
			printf("BOOL ");
		else if (target.simple.type == VOID)
			printf("VOID ");

		// TODO: this should append/create an array
		// if (target.simple.type == SIGNED)
		// 	printf("SIGNED ");
		// else if (target.simple.type == UNSIGNED)
		// 	printf("UNSIGNED ");
		// else if (target.simple.type == SHORT)
		// 	printf("SHORT ");
		// else if (target.simple.type == LONG)
		// 	printf("LONG ");
	}
	else if (target.type == C_PTR) {
		printf("C_PTR: ");
		print_type(*target.ptr.to, false);
	}
	else if (target.type == C_ADR) {
		printf("C_ADR: ");
		print_type(*target.adr.to, false);
	}
	else if (target.type == C_ARR) {
		printf("C_ARR (%d): ", target.arr.size);
		print_type(*target.arr.of, false);
	}
	else if (target.type == C_ENM) {
		printf("C_ENM: ");
		for (int i = 0; i < target.enm.member_count; i++)
			printf(" { %s: %d }", ((enm_member*)target.enm.members->items[i])->key, ((enm_member*)target.enm.members->items[i])->val);
	}
	else if (target.type == C_STRT) {
		printf("C_STRT: ");
		for (int i = 0; i < target.strt.member_count; i++) {
			printf(" { %s: (", ((strt_member*)target.strt.members->items[i])->key);
			print_type(*((strt_member*)target.strt.members->items[i])->val, false);
			printf(") }");
		}
	}
	else if (target.type == C_UNN) {
		printf("C_UNN: ");
		for (int i = 0; i < target.unn.member_count; i++) {
			printf(" { %s: (", ((strt_member*)target.unn.members->items[i])->key);
			print_type(*((strt_member*)target.unn.members->items[i])->val, false);
			printf(") }");
		}
	}
	else if (target.type == C_FN) {
		printf("C_FN: ");
		print_type(*target.fn.type, false);
		for (int i = 0; i < target.fn.arg_count; i++) {
			printf(" { %s: (", ((fn_arg*)target.fn.args->items[i])->name);
			print_type(*((fn_arg*)target.fn.args->items[i])->type, false);
			printf(") }");
		}
	}
	else printf("UNKNOWN DATA TYPE... (%d)", target.type);

	if (newline) printf("\n");
}

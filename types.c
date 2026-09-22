#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "types.h"
#include "./utils/core.h"

char *c_type_type_names[] = {
	"C_SIMPLE",
	"C_PTR",
	"C_ADR",
	"C_ARR",
	"C_ENM",
	"C_STRT",
	"C_UNN",
	"C_FN",
};

int c_type_simple_names_len = 6;
char *c_type_simple_names[6] = {
	"int",
	"float",
	"double",
	"char",
	"bool",
	"void",
};

bool is_type_mod(const char *str) {
	return
		strcmp(str, "stack") == 0 ||
		strcmp(str, "heap") == 0 ||
		strcmp(str, "mutable") == 0 ||
		strcmp(str, "const") == 0 ||
		strcmp(str, "signed") == 0 ||
		strcmp(str, "unsigned") == 0 ||
		strcmp(str, "short") == 0 ||
		strcmp(str, "long") == 0;
}

bool is_cmplx_type(const char *str) {
	return
		strcmp(str, "struct") == 0 ||
		strcmp(str, "enum") == 0 ||
		strcmp(str, "union") == 0;
}

bool is_type(const char *str) {
	return
		strcmp(str, "int") == 0 ||
		strcmp(str, "float") == 0 ||
		strcmp(str, "double") == 0 ||
		strcmp(str, "char") == 0 ||
		strcmp(str, "bool") == 0 ||
		strcmp(str, "void") == 0;
}

c_type *get_type(const char *str) {
	c_type *result = malloc(sizeof(c_type));
	if (strcmp(str, "int") == 0) {
		result->type = C_SIMPLE;
		result->simple.type = INT;
	}
	else if (strcmp(str, "float") == 0) {
		result->type = C_SIMPLE;
		result->simple.type = FLOAT;
	}
	else if (strcmp(str, "double") == 0) {
		result->type = C_SIMPLE;
		result->simple.type = DOUBLE;
	}
	else if (strcmp(str, "char") == 0) {
		result->type = C_SIMPLE;
		result->simple.type = CHAR;
	}
	else if (strcmp(str, "bool") == 0) {
		result->type = C_SIMPLE;
		result->simple.type = BOOL;
	}
	else if (strcmp(str, "void") == 0) {
		result->type = C_SIMPLE;
		result->simple.type = VOID;
	}
	else {
		fprintf(stderr, "%s : ", str);
		raise_err("Failed to parse type that does not exist (not 'int', 'float', 'double', 'char' or 'bool')");
	}

	return result;
}

c_type_simple_modifier get_type_mod(char *target) {
	if (strcmp(target, "stack") == 0)
		return STACK;
	else if (strcmp(target, "heap") == 0)
		return HEAP;
	else if (strcmp(target, "mutable") == 0)
		return MUTABLE;
	else if (strcmp(target, "signed") == 0)
		return SIGNED;
	else if (strcmp(target, "unsigned") == 0)
		return UNSIGNED;
	else if (strcmp(target, "short") == 0)
		return SHORT;
	else if (strcmp(target, "long") == 0)
		return LONG;
	else {
		fprintf(stderr, "%s : ", target);
		raise_err("Failed to parse type modifier that does not exist (not 'stack', 'heap', 'mutable', 'signed', 'unsigned', 'short' or 'long')");
	}
	return NONE;
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

		if (target.simple.modifiers && target.simple.modifiers->size > 0) {
			printf("( ");
			int i;
			c_type_simple_modifier *mod;
			RARRAY_FOREACH(mod, target.simple.modifiers, i) {
				switch (*mod) {
					case NONE:
						printf("NONE ");
						break;
					case SIGNED:
						printf("SIGNED ");
						break;
					case UNSIGNED:
						printf("UNSIGNED ");
						break;
					case STACK:
						printf("STACK ");
						break;
					case HEAP:
						printf("HEAP ");
						break;
					case SHORT:
						printf("SHORT ");
						break;
					case LONG:
						printf("LONG ");
						break;
					case MUTABLE:
						printf("MUTABLE ");
				}
			}
			printf(")");
		}
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
		if (target.arr.size_identifier == NULL)
			printf("C_ARR (%d): ", target.arr.size_int);
		else
			printf("C_ARR (IDENTIFIER: '%s'): ", target.arr.size_identifier);
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

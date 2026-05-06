#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include "arrays.h"

typedef struct {
	const char *name;
	struct c_type *type;
} fn_arg;

typedef struct {
	char *key;
	struct c_type *val;
} strt_member;

typedef struct {
	char *key;
	int val;
} enm_member;

typedef enum { // TODO - not implemented (+ we need to do const as well)
	NONE,
	SIGNED,
	UNSIGNED,
	SHORT,
	LONG,
} c_type_simple_modifier;

typedef struct c_type {
	enum c_type_type {
		C_SIMPLE,
		C_PTR,
		C_ADR,
		C_ARR,
		C_ENM,
		C_STRT,
		C_UNN,
		C_FN,
	} type;
	union {
		struct c_type_simple {
			enum {
				INT,
				FLOAT,
				DOUBLE,
				CHAR,
				BOOL,
				VOID,
			} type;
			rarray *modifiers; // c_type_simple_modifier
		} simple;
		struct {
			struct c_type *to;
		} ptr;
		struct {
			struct c_type *to;
		} adr;
		struct {
			struct c_type *of;
			int size; // -1 = inferred
		} arr;
		struct {
			int member_count;
			rarray *members; // enm_member
			const char *name;
		} enm;
		struct {
			int member_count;
			rarray *members; // strt_member
			const char *name;
		} strt, unn;
		struct {
			int arg_count;
			rarray *args; // fn_arg
			struct c_type *type;
		} fn;
	};
} c_type;

bool is_type(const char *str);
c_type get_type(const char *str);
void print_type(c_type target, bool newline);

#endif

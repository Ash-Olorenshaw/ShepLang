#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../string_utils.h"
#include "../utils/arrays.h"
#include "../utils/core.h"
#include "../types.h"
#include "./types.h"
#include "./raw.h"

void process_compound_type(const char *elements, c_type **result, enum c_type_type target_type) {
	// TODO: we should check if the union/struct has been defined yet, etc
	rarray **target = target_type == C_UNN ? &(*result)->unn.members : &(*result)->strt.members;

	*target = rarray_create(10, sizeof(strt_member*));
	int i, start;

	start = 0;

	for (i = 0; i < (int) strlen(elements); i++) {
		const char c = elements[i];
		if (c == '{') start++;
		if (c == ';') {
			char* line = substr(elements, start, i + 1);
			if (line == NULL) continue;
			strt_member *member = malloc(sizeof(strt_member));
			member->key = NULL;
			member->val = get_var_type(get_var_elems(line), str_find(line, ")(") != -1, &member->key);
			print_type(*member->val, true);
			start = i + 1;
			if (!rarray_add(*target, member))
				raise_err("Failed to add item to union/struct type array.");
			free(line);
		}
	}


	// TODO - we should handle below better I reckon
	if (target_type == C_UNN)
		(*result)->unn.member_count = (*target)->size;
	else {
		(*result)->strt.member_count = (*target)->size;
		printf(" GENERATING STRT : %d %d \n", (*result)->strt.member_count, (*target)->size);
	}
}

void process_enum_type(const char *elements, c_type **result) {
	// (*result)->enm.members = rarray_create(10, sizeof(enm_member*));
	// int i, start;
	//
	// start = 0;
	//
	// int elem_count = strlen(elements);
	// for (i = 0; i < elem_count; i++) {
	// 	const char c = elements[i];
	// 	if (c == '{') start++;
	// 	if (c == ',' || i == elem_count) {
	// 		char* line = substr(elements, start, i + 1);
	// 		if (line == NULL) continue;
	// 		strt_member *unn_item = malloc(sizeof(strt_member));
	// 		unn_item->key = NULL;
	// 		unn_item->val = get_var_type(get_var_elems(line), str_find(line, ")(") != -1, &unn_item->key);
	// 		start = i;
	// 		if (!rarray_add((*result)->unn.members, unn_item))
	// 			raise_err("Failed to add item to union type array.");
	// 		free(line);
	// 	}
	// }
	//
	//
	// (*result)->unn.member_count = (*result)->unn.members->size;
}

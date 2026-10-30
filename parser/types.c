#include <stdbool.h>
#include <string.h>
#include "../utils/arrays.h"
#include "../utils/core.h"
#include "../tokeniser.h"
#include "./vars.h"

// NOTE - does not match "in/out/inout"
bool types_match(c_type *type_1, c_type *type_2, int depth);
bool types_match(c_type *type_1, c_type *type_2, int depth) {
	if (depth > 100)
		raise_err("Failed to parse type with over 100 levels of redirection (structs, arrays, pointers, etc).");
	bool match = false;
	if (type_1->type == type_2->type) {
		switch (type_1->type) {
			case C_SIMPLE:
				if (type_1->simple.type == type_2->simple.type && type_1->simple.modifiers->size == type_2->simple.modifiers->size) {
					bool all_mods_match = true;
					int i;
					c_type_simple_modifier *mod;
					bool type_2_includes = false;
					RARRAY_FOREACH(mod, type_1->simple.modifiers, i) {
						RARRAY_CONTAINS(
							type_2->simple.modifiers,
							*mod,
							c_type_simple_modifier,
							type_2_includes
						);
						if (!type_2_includes) {
							all_mods_match = false;
							break;
						}
						type_2_includes = false;
					}
					if (all_mods_match)
						match = true;
				}
				break;
			case C_PTR:
				bool all_mods_match = true;
				int i;
				c_type_ptr_modifier *mod;
				bool type_2_includes = false;
				RARRAY_FOREACH(mod, type_1->ptr.modifiers, i) {
					if (*mod != IN && *mod != INOUT && *mod != OUT) {
						RARRAY_CONTAINS(
							type_2->ptr.modifiers,
							*mod,
							c_type_ptr_modifier,
							type_2_includes
						);
						if (!type_2_includes) {
							all_mods_match = false;
							break;
						}
						type_2_includes = false;
					}
				}
				if (all_mods_match)
					match = types_match(type_1->ptr.to, type_2->ptr.to, depth + 1);
				break;
			case C_ADR:
				match = types_match(type_1->adr.to, type_2->adr.to, depth + 1);
				break;
			case C_ARR:
				if (
						(
							(
								type_1->arr.size_identifier != NULL && type_2->arr.size_identifier != NULL &&
								strcmp(type_1->arr.size_identifier, type_2->arr.size_identifier) == 0
							) ||
							(
								type_1->arr.size_identifier == NULL && type_2->arr.size_identifier == NULL &&
								type_1->arr.size_int == type_2->arr.size_int
							)
						)
				   ) {
					match = types_match(type_2->arr.of, type_1->arr.of, depth + 1);
				}
				break;
			case C_ENM:
				if (type_1->enm.member_count == type_2->enm.member_count) {
					enm_member *member;
					int i;
					bool all_members_match = true;
					RARRAY_FOREACH(member, type_1->enm.members, i) {
						enm_member *member_2 = type_2->strt.members->items[i];
						if (strcmp(member->key, member_2->key) != 0 || member->val != member_2->val) {
							all_members_match = false;
							break;
						}
					}
					if (all_members_match)
						match = true;
				}
				break;
			case C_STRT:
				if (type_1->strt.member_count == type_2->strt.member_count) {
					strt_member *member;
					int i;
					bool all_members_match = true;
					RARRAY_FOREACH(member, type_1->strt.members, i) {
						strt_member *member_2 = type_2->strt.members->items[i];
						if (strcmp(member->key, member_2->key) != 0 || types_match(member->val, member_2->val, depth + 1)) {
							all_members_match = false;
							break;
						}
					}
					if (all_members_match)
						match = true;
				}
				break;
			case C_UNN:
				if (type_1->unn.member_count == type_2->unn.member_count) {
					strt_member *member;
					int i;
					bool all_members_match = true;
					RARRAY_FOREACH(member, type_1->unn.members, i) {
						strt_member *member_2 = type_2->unn.members->items[i];
						if (strcmp(member->key, member_2->key) != 0 || types_match(member->val, member_2->val, depth + 1)) {
							all_members_match = false;
							break;
						}
					}
					if (all_members_match)
						match = true;
				}
				break;
			case C_FN:
				if (types_match(type_1->fn.type, type_2->fn.type, depth + 1) && type_1->fn.arg_count == type_2->fn.arg_count) {
					fn_arg *arg;
					int i;
					bool all_args_match = true;
					RARRAY_FOREACH(arg, type_1->fn.args, i) {
						if (!types_match(arg->type, ((fn_arg *)type_2->fn.args->items[i])->type, depth + 1)) {
							all_args_match = false;
							break;
						}
					}
					if (all_args_match)
						match = true;
				}
			break;
		}
	}
	return match;
}

c_type *get_tkns_type(rarray *tkns, int depth);
c_type *get_tkns_type(rarray *tkns, int depth) {
	bool prev_was_value;
	int i;
	tkn *current_tkn;
	tkn *nxt_tkn;
	c_type *current_type = malloc(sizeof(c_type));
	RARRAY_FOREACH(current_tkn, tkns, i) {
		if (i < tkns->size - 1)
			nxt_tkn = tkns->items[i + 1];
		else nxt_tkn = NULL;
		prev_was_value = false;

		switch (current_tkn->type) {
			case IDENTIFIER_IF:
			case IDENTIFIER_ELSE:
			case IDENTIFIER_ELSEIF:
			case IDENTIFIER_WHILE:
			case IDENTIFIER_FOR:
			case IDENTIFIER_RETURN:
			case IDENTIFIER_BREAK:
			case IDENTIFIER_CONTINUE:
			case IDENTIFIER_SWITCH:
			case IDENTIFIER_CASE:
			case IDENTIFIER_DEFAULT:
			case IDENTIFIER_GOTO:
				raise_err("Unexpected control statement found");
			case IDENTIFIER_SIZEOF:
				prev_was_value = true;
				if (nxt_tkn != NULL && nxt_tkn->type == CONTAINER) {
					current_type->type = C_SIMPLE;
					current_type->simple.type = INT;
					i++;
				}
				else
					raise_err("Sizeof operator acts like a function, expected () following it.");
				break;
			case LITERAL_CHAR:
				if (prev_was_value)
					raise_err("Expect operator to precede literal '%s'.", current_tkn->content);
				prev_was_value = true;
				if (i == 0) {
					current_type->type = C_SIMPLE;
					current_type->simple.type = CHAR;
					// TODO: current_type->simple.modifiers
					// Mark everything as inout until you get to a var
				}
				else if (
						current_type->type == C_SIMPLE && (
							current_type->simple.type == CHAR ||
							current_type->simple.type == FLOAT ||
							current_type->simple.type == DOUBLE ||
							current_type->simple.type == INT
						)
					) {
					if (current_type->simple.type == CHAR)
						current_type->simple.type = INT;
					break;
				}
				else {
					raise_err("Unexpected char literal.");
				}
			case LITERAL_NUMBER:
				if (prev_was_value)
					raise_err("Expect operator to precede literal '%s'.", current_tkn->content);
				prev_was_value = true;
				if (i == 0) {
					current_type->type = C_SIMPLE;
					current_type->simple.type = INT;
				}
				else if (
						current_type->type == C_SIMPLE && (
							current_type->simple.type == CHAR ||
							current_type->simple.type == FLOAT ||
							current_type->simple.type == DOUBLE ||
							current_type->simple.type == INT
						)
					) {
					if (current_type->simple.type == CHAR)
						current_type->simple.type = INT;
					break;
				}
				else {
					raise_err("Unexpected integer literal.");
				}
			case LITERAL_STRING:
				if (prev_was_value)
					raise_err("Expect operator to precede literal '%s'.", current_tkn->content);
				prev_was_value = true;
				if (i == 0) {
					current_type->type = C_PTR;
					current_type->ptr.to = malloc(sizeof(c_type));
					current_type->ptr.to->type = C_SIMPLE;
					current_type->ptr.to->simple.type = CHAR;
				}
				else if (
						current_type->type == C_PTR &&
						current_type->ptr.to->type == C_SIMPLE &&
						current_type->ptr.to->simple.type == CHAR
					) {
					break;
				}
				else {
					raise_err("Unexpected string literal.");
				}
			case IDENTIFIER:
				if (prev_was_value)
					raise_err("Expect operator to precede identifier '%s'.", current_tkn->content);
				prev_was_value = true;
				identifier *found_identifier = scope_find(current_tkn->content);
				if (found_identifier == NULL)
					raise_err("Referenced identifier '%s' could not be found.", found_identifier->name);
				if (found_identifier->freed)
					raise_err("Attempted to use var '%s' after it was freed.", found_identifier->name);
				if (i == 0)
					current_type = found_identifier->type;
				else if (!types_match(current_type, found_identifier->type, 0)) {
					if (current_type->type == C_SIMPLE && found_identifier->type->type == current_type->type) {
						if (
								(
									current_type->simple.type == FLOAT ||
									current_type->simple.type == DOUBLE ||
									current_type->simple.type == CHAR ||
									current_type->simple.type == INT
								) &&
								(
									found_identifier->type->type == FLOAT ||
									found_identifier->type->type == DOUBLE ||
									found_identifier->type->type == CHAR ||
									found_identifier->type->type == INT
								)
							)
							break;
					}
					raise_err("Attempted to use var '%s' which has a type that doesn't match surrounding values.", found_identifier->name);
				}
				break;
			case BLOCK_START:
			case BLOCK_END:
				raise_err("Unexpected {} in ().");
			case CONTAINER:
				if (depth > 50)
					raise_err("Max depth reached with nested brackets.");
				current_type = get_tkns_type(current_tkn->tkn_content, depth + 1);
				break;
			case OPERATOR_SLICE:
				prev_was_value = true;
				if (current_tkn->type != C_ARR && current_tkn->type != C_PTR) {
					raise_err("Incorrect type for slice operator.");
				}
				else if (current_tkn->type == C_ARR)
					current_type = current_type->arr.of;
				else
					current_type = current_type->ptr.to;
			case OPERATOR_ASSIGN:
				if (strcmp(current_tkn->content, "=") == 0)
					break;
				else if (
						i > 0 &&
						current_type->type == C_SIMPLE && (
							current_type->simple.type == FLOAT ||
							current_type->simple.type == DOUBLE ||
							current_type->simple.type == CHAR ||
							current_type->simple.type == INT
						)
					)
					break;
				else
					raise_err("Assignment operator with the wrong types.");
			case OPERATOR:
				if (
						i > 0 && current_type->type == C_SIMPLE && (
							current_type->simple.type == FLOAT ||
							current_type->simple.type == DOUBLE ||
							current_type->simple.type == CHAR ||
							current_type->simple.type == INT
						) && (
							strcmp(current_tkn->content, "+") == 0 ||
							strcmp(current_tkn->content, "-") == 0 ||
							strcmp(current_tkn->content, "*") == 0 ||
							strcmp(current_tkn->content, "/") == 0 ||
							strcmp(current_tkn->content, "%") == 0 ||
							strcmp(current_tkn->content, ">") == 0 ||
							strcmp(current_tkn->content, "<") == 0 ||
							strcmp(current_tkn->content, ">=") == 0 ||
							strcmp(current_tkn->content, "<=") == 0 ||
							strcmp(current_tkn->content, "&") == 0 ||
							strcmp(current_tkn->content, "|") == 0 ||
							strcmp(current_tkn->content, "^") == 0 ||
							strcmp(current_tkn->content, "<<") == 0 ||
							strcmp(current_tkn->content, ">>") == 0
						)
					) {
					if (current_type->simple.type == CHAR)
						current_type->simple.type = INT;
				}
				else if (strcmp(current_tkn->content, "!") == 0) {
					current_type->type = C_SIMPLE;
					current_type->simple.type = BOOL;
				}
				else if (
							i > 0 && nxt_tkn != NULL && (
								strcmp(current_tkn->content, "||") == 0 ||
								strcmp(current_tkn->content, "&&") == 0 ||
								strcmp(current_tkn->content, "==") == 0 ||
								strcmp(current_tkn->content, "!=") == 0
							)
					) {
					current_type->type = C_SIMPLE;
					current_type->simple.type = BOOL;
				}
				else if (strcmp(current_tkn->content, "*") == 0) {
					if (prev_was_value)
						raise_err("Expect operator before '*' pointer operator.");
					if (current_type->type == C_PTR) {
						c_type *original_ptr = current_type;
						current_type = current_type->ptr.to;
						free(original_ptr);
					}
					else
						raise_err("Unable to dereference non-pointer type.");
				}
				else if (strcmp(current_tkn->content, "&") == 0) {
					if (prev_was_value)
						raise_err("Expect operator before '&' address operator.");
					c_type *ptr_to = current_type;
					current_type = malloc(sizeof(c_type));
					current_type->type = C_PTR;
					current_type->ptr.to = ptr_to;
				}
				else if (strcmp(current_tkn->content, "?") == 0 || strcmp(current_tkn->content, ":") == 0) {
					raise_err("Ternaries not currently implemented.");
				}
				else if (
							current_type->type == C_SIMPLE &&
							current_type->simple.type == INT &&
							strcmp(current_tkn->content, ".") == 0
						) {
					if (prev_was_value && nxt_tkn != NULL && nxt_tkn->type == LITERAL_NUMBER) {
						current_type->type = C_SIMPLE;
						current_type->simple.type = FLOAT;
						i++;
					}
					else
						raise_err("Incomplete float. Floats require a digit either side of the '.'");
				}
				else if (
							(
								current_type->type == C_ENM ||
								current_type->type == C_STRT ||
								current_type->type == C_UNN
							) && (
								strcmp(current_tkn->content, "->") == 0 ||
								strcmp(current_tkn->content, ".") == 0
							)
						) {
					raise_err("Enum, struct, union member access not currently implemented.");
				}
				else
					raise_err("Operator %s used with incorrect types.", current_tkn->content);
				break;

		}
	}
	return current_type;
}

bool compare_function_args(rarray *fn_args, tkn *container_tkn) {
	int j, prev = 0, fn_arg_i = 0;
	tkn *provided_arg;
	bool all_args_match = true;
	RARRAY_FOREACH(provided_arg, container_tkn->tkn_content, j) {
		if (provided_arg->type == OPERATOR && strcmp(",", provided_arg->content)) {
			if (j < container_tkn->tkn_content->size - 1 && prev < j - 1) {
				rarray *provided_arg_tkns = rarray_slice(container_tkn->tkn_content, prev, j - 1);
				prev = j + 1;
				c_type *provided_arg_type = get_tkns_type(provided_arg_tkns, 0);
				if (fn_arg_i >= fn_args->size)
					raise_err("Too many arguments provided to function.");
				fn_arg *current_arg = fn_args->items[fn_arg_i++];
				if (types_match(provided_arg_type, current_arg->type, 0)) {
					if (current_arg->type->type == C_PTR) {
						bool contains_modifier_in = false;
						bool contains_modifier_out = false;
						bool contains_modifier_inout = false;

						RARRAY_CONTAINS(current_arg->type->ptr.modifiers, IN, c_type_ptr_modifier, contains_modifier_in);

						if (contains_modifier_in) {
							RARRAY_CONTAINS(provided_arg_type->ptr.modifiers, IN, c_type_ptr_modifier, contains_modifier_in);
							if (contains_modifier_in) {
								if (provided_arg_tkns->size == 1) {
									tkn *first_tkn = provided_arg_tkns->items[0];
									if (first_tkn->type == IDENTIFIER) {
										identifier *found = scope_find(first_tkn->content);
										if (found->freed)
											raise_err("Var '%s' supplied to 'in' parameter of function when it has already been freed.", found->name);
										found->freed = true;
									}
								}
								else raise_err("Unable to find identifier of type 'in' to provide to function 'in' parameter.");
							}
						}
						else {
							RARRAY_CONTAINS(current_arg->type->ptr.modifiers, INOUT, c_type_ptr_modifier, contains_modifier_inout);
							if (contains_modifier_inout) {
								RARRAY_CONTAINS(provided_arg_type->ptr.modifiers, INOUT, c_type_ptr_modifier, contains_modifier_inout);
								if (!contains_modifier_inout) {
									printf("WARN - non 'out' point passed to 'out' parameter. Input may be discarded.");
								}
							}
							else {
								RARRAY_CONTAINS(current_arg->type->ptr.modifiers, OUT, c_type_ptr_modifier, contains_modifier_out);
								if (contains_modifier_out) {
									RARRAY_CONTAINS(provided_arg_type->ptr.modifiers, OUT, c_type_ptr_modifier, contains_modifier_out);
									if (!contains_modifier_out) {
										printf("WARN - non 'out' point passed to 'out' parameter. Input may be discarded.");
									}
								}
							}
						}

					}
				}
				else
					all_args_match = false;
			}
			else
				raise_err("Function argument list cannot have an empty argument provided.");
		}
	}
	return all_args_match;
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../utils/strings/type.h"
#include "../utils/strings/utils.h"
#include "../utils/arrays.h"
#include "../utils/core.h"
#include "../tokeniser.h"
#include "../types.h"
#include "./vars.h"
#include "./types.h"

FILE *prototypeFile = NULL;
FILE *outputFile = NULL;

// printf("prototypeFile : %s\n", prototypeFile == NULL ? "null" : "other");
// printf("Writing:");
// printf(__VA_ARGS__);
// printf("\n");
#define prototype_write(...) { \
	if (prototypeFile != NULL) \
		fprintf(prototypeFile, __VA_ARGS__); \
}

// printf("outputFile : %s\n", outputFile == NULL ? "null" : "other");
// printf("Writing:");
// printf(__VA_ARGS__);
// printf("\n");
#define result_write(...) { \
	if (outputFile != NULL) \
		fprintf(outputFile, __VA_ARGS__); \
}

void write_variable(c_type *var_type, char *name, bool proto, bool type_only) {
	string_builder *closing_section = string_builder_create(10);
	string_builder *start_section = string_builder_create(10);
	string_builder *mod_section = string_builder_create(10);
	identifier *new_identifier = malloc(sizeof(identifier));

	new_identifier->type = var_type;
	new_identifier->name = strdup(name);

	while(var_type != NULL) {
		bool signed_set = false;
		bool length_set = false;
		int i;
		switch (var_type->type) {
			case C_SIMPLE:
				str_rev(closing_section->string);
				char *type_name;
				if ((int) var_type->simple.type < c_type_simple_names_len)
					type_name = c_type_simple_names[var_type->simple.type];
				else {
					raise_err("UNKNOWN var type: %d\n", var_type->simple.type);
				}

				new_identifier->mutable = IDENTIFIER_UNKNOWN_MUTABILITY;
				c_type_simple_modifier *simple_mod;
				RARRAY_FOREACH(simple_mod, var_type->simple.modifiers, i) {
					switch (*simple_mod) {
						case NONE:
							// UNREACHABLE
							break;
						case SIGNED:
							if (signed_set)
								raise_err("Var definition cannot specify multiple signed states, pick one: (unsigned, signed)");
							signed_set = true;
							string_builder_add_s(mod_section, "signed ");
							break;
						case UNSIGNED:
							if (signed_set)
								raise_err("Var definition cannot specify multiple signed states, pick one: (unsigned, signed)");
							signed_set = true;
							string_builder_add_s(mod_section, "unsigned ");
							break;
						case SHORT:
							if (length_set)
								raise_err("Var definition cannot specify multiple lengths, pick one: (long, short)");
							length_set = true;
							string_builder_add_s(mod_section, "short ");
							break;
						case LONG:
							if (length_set)
								raise_err("Var definition cannot specify multiple lengths, pick one: (long, short)");
							length_set = true;
							string_builder_add_s(mod_section, "long ");
							break;
						case MUTABLE:
							if (new_identifier->mutable == IDENTIFIER_UNKNOWN_MUTABILITY)
								new_identifier->mutable = IDENTIFIER_MUTABLE;
							else
								raise_err("Var definition cannot repeat mutability specifiers");
					}
				}
				if (new_identifier->mutable == IDENTIFIER_UNKNOWN_MUTABILITY)
					new_identifier->mutable = IDENTIFIER_IMMUTABLE;
				result_write("%s %s %s%s%s", mod_section->string, type_name, start_section->string, name, closing_section->string);
				if (proto)
					prototype_write("%s %s %s%s%s", mod_section->string, type_name, start_section->string, name, closing_section->string);
				var_type = NULL;

				break;
			case C_PTR:

				new_identifier->ownership = IDENTIFIER_UNKNOWN_OWNERSHIP;
				new_identifier->location = IDENTIFIER_UNKNOWN;
				new_identifier->freed = false;

				c_type_ptr_modifier *ptr_mod;
				RARRAY_FOREACH(ptr_mod, var_type->ptr.modifiers, i) {
					switch (*ptr_mod) {
						case IN:
							if (new_identifier->ownership == IDENTIFIER_UNKNOWN_OWNERSHIP)
								new_identifier->ownership = IDENTIFIER_IN;
							else
								raise_err("Var definition cannot repeat ownership specifiers, pick one: (in, out, inout)");
							break;
						case INOUT:
							if (new_identifier->ownership == IDENTIFIER_UNKNOWN_OWNERSHIP)
								new_identifier->ownership = IDENTIFIER_INOUT;
							else
								raise_err("Var definition cannot repeat ownership specifiers, pick one: (in, out, inout)");
							break;
						case OUT:
							if (new_identifier->ownership == IDENTIFIER_UNKNOWN_OWNERSHIP)
								new_identifier->ownership = IDENTIFIER_OUT;
							else
								raise_err("Var definition cannot repeat ownership specifiers, pick one: (in, out, inout)");
							break;
						case STACK:
							if (new_identifier->location == IDENTIFIER_UNKNOWN)
								new_identifier->location = IDENTIFIER_STACK_ALLOCATED;
							else
								raise_err("Var definition cannot repeat allocation specifiers, pick one: (heap, stack)");
							break;
						case HEAP:
							if (new_identifier->location == IDENTIFIER_UNKNOWN)
								new_identifier->location = IDENTIFIER_HEAP_ALLOCATED;
							else
								raise_err("Var definition cannot repeat allocation specifiers, pick one: (heap, stack)");
							break;
					}
				}

				if (new_identifier->ownership == IDENTIFIER_UNKNOWN_OWNERSHIP)
					new_identifier->ownership = IDENTIFIER_INOUT;

				string_builder_add_c(start_section, '*');
				string_builder_add_c(start_section, '(');
				string_builder_add_c(closing_section, ')');
				var_type = var_type->ptr.to;
				break;
			case C_ADR:
				raise_err("Cannot have the address operator '&' in a variable defintion");
			case C_ARR:
				string_builder_add_c(start_section, '(');
				if (!type_only)
					string_builder_add_c(closing_section, ']');

				if (var_type->arr.size_identifier != NULL) {
					if (scope_find(var_type->arr.size_identifier)) {
						// check type
						string_builder_add_s(closing_section, var_type->arr.size_identifier);
					}
					else {
						raise_err("Cannot set an array length with unknown identifier '%s'", var_type->arr.size_identifier);
					}
				}
				else if (var_type->arr.size_int > 0) {
					char *intstr = int_to_str(var_type->arr.size_int);
					string_builder_add_s(closing_section, intstr);
					free(intstr);
				}
				else if (type_only) raise_err("Cannot infer array size for heap-allocated array\n");
				if (type_only)
					string_builder_add_c(closing_section, '*');
				else
					string_builder_add_c(closing_section, '[');
				string_builder_add_c(closing_section, ')');
				var_type = var_type->arr.of;
				break;
			default:
				// printf("ERROR WITH WRITING TYPE: %s\n", c_type_type_names[var_type->type]);
				raise_err("Failed to write type for var: %s\n", name);
				break;
			// case C_ENM:
			// 	result_write("enum {");
			// 	for (int i = 0; i < var_type->enm.member_count; i++)
			// 		result_write(" %s = %d", ((enm_member*)var_type->enm.members->items[i])->key, ((enm_member*)var_type->enm.members->items[i])->val);
			// 	result_write("}")
			// 	break;
			case C_STRT:
				result_write("struct {");
				strt_member *strt_item;
				RARRAY_FOREACH(strt_item, var_type->strt.members, i) {
					result_write("%s", strt_item->key);
					write_variable(strt_item->val, strt_item->key, false, false);
					result_write(";")
				}
				result_write("}");
				break;
			case C_UNN:
				result_write("union {");
				strt_member *unn_item;
				RARRAY_FOREACH(unn_item, var_type->strt.members, i) {
					result_write("%s", unn_item->key);
					write_variable(unn_item->val, unn_item->key, false, false);
					result_write(";")
				}
				result_write("}");
				break;
			// case C_FN:
			// 	printf("C_FN: ");
			// 	print_type(*var_type->fn.type, false);
			// 	for (int i = 0; i < var_type->fn.arg_count; i++) {
			// 		printf(" { %s: (", ((fn_arg*)var_type->fn.args->items[i])->name);
			// 		print_type(*((fn_arg*)var_type->fn.args->items[i])->type, false);
			// 		printf(") }");
			// 	}
		}
	}

	add_to_scope(new_identifier);

	// result_write(target->var_definition.name);
	// printf("VAR DEFINITION: { (%s) : ", );
	// print_type(*line->var_definition.type, false);
	// printf(" } => \n");
	// if (line->var_definition.simple)
	// 	RARRAY_FOREACH(elem, line->var_definition.val, i) {
	// 		print_tkn(elem, depth + 1);
	// 	}
	// else {
	// 	RARRAY_FOREACH(elem_line, line->var_definition.val_lines, i) {
	// 		print_tokenised_line(elem_line, depth + 1);
	// 	}
	// }

	string_builder_free(&closing_section);
	string_builder_free(&start_section);
	string_builder_free(&mod_section);
}

rarray *tkns_to_statements(rarray *tkns) {
	rarray *statements = rarray_create(5, sizeof(tkn_line));
	rarray *statement = rarray_create(5, sizeof(tkn));
	int i;
	tkn *target_tkn;
	RARRAY_FOREACH(target_tkn, tkns, i) {
		rarray_add(statement, target_tkn);
		if ((target_tkn->type == OPERATOR && strcmp(target_tkn->content, ";")) || i == tkns->size - 1) {
			tkn_line *new_line = malloc(sizeof(tkn_line));
			new_line->statement.tkns = statement;
			rarray_add(statements, new_line);
			if (i < tkns->size - 1)
				statement = rarray_create(5, sizeof(tkn));
		}
	}
	return statements;
}

void write_statement(tkn_line *target);
void write_tkn(tkn *target);
void write_tkn(tkn *target) {
	if (target->type == CONTAINER || target->type == OPERATOR_SLICE) {
		result_write("%s", target->type == CONTAINER ? "(" : "[");
		int i;
		tkn_line *sub_stmnt;
		rarray *statements = tkns_to_statements(target->tkn_content);
		RARRAY_FOREACH(sub_stmnt, statements, i)
			write_statement(sub_stmnt);
		result_write("%s", target->type == CONTAINER ? ")" : "]");
	}
	else if (target->type == BLOCK_START) {
		result_write("{");
	}
	else if (target->type == BLOCK_END) {
		result_write("}");
	}
	else if (target->content) {
		result_write("%s", target->content);
	}
	else {
		raise_err("Failed to write: %s (%s)\n", target->content, tkn_type_str[target->type]);
	}
}

void write_statement(tkn_line *target) {
	if (target != NULL) {
		tkn *elem;
		int i;
		RARRAY_FOREACH(elem, target->statement.tkns, i) {
			if (elem->type == CONTAINER && i > 0) {
				tkn *last_tkn = target->statement.tkns->items[i-1];
				if (last_tkn->type == IDENTIFIER) {
					identifier *found = scope_find(last_tkn->content);
					if (!found)
						raise_err("Failed to find identifier '%s' in current scope", last_tkn->content);
					if (found->type->type == C_FN) {
						if (!compare_function_args(found->type->fn.args, elem))
							raise_err("Function called with incorrect arguments: '%s'", last_tkn->content);
					}
				}
			}
			else if (elem->type == OPERATOR_ASSIGN) {
				if (i > 0) {
					tkn *last_tkn = target->statement.tkns->items[i-1];
					if (last_tkn->type != IDENTIFIER) {
						raise_err("Assignment operators require an identifier on the left side of the operator.");
					}
					else {
						identifier *found = scope_find(last_tkn->content);
						if (found) {
							if (found->mutable != IDENTIFIER_MUTABLE) {
								fprintf(stderr, "%s : ", found->name);
								raise_err("Reassigning immutable variable. Mark variable with 'mutable' to be able to change it.");
							}
						}
						else {
							raise_err("Failed to find var identifier '%s'", last_tkn->content);
						}
					}
				}
			}
			write_tkn(elem);
			result_write(" ");
		}
	}
}

void write_block(tkn_line *target) {
	switch (target->block.type->type) {
		case IDENTIFIER_IF:
			result_write("if");
			write_tkn(target->block.content);
			break;
		case IDENTIFIER_ELSEIF:
			result_write("else if");
			write_tkn(target->block.content);
			break;
		case IDENTIFIER_ELSE:
			result_write("else");
			break;
		case IDENTIFIER_FOR:
			result_write("for");
			write_tkn(target->block.content);
			break;
		case IDENTIFIER_WHILE:
			result_write("while");
			write_tkn(target->block.content);
			break;
		default:
			raise_err("Currently unsupport block type: %s\n", tkn_names[target->block.type->type]);
	}
}

void write_function(tkn_line *target) {
	string_builder *mod_section = string_builder_create(10);
	string_builder *start_section = string_builder_create(10);
	char *type_name;
	c_type *func_type = target->func_definition.type;
	if (func_type->type == C_FN)
		func_type = func_type->fn.type;
	else
		raise_err("incomplete function signature does not contain arguments (%s).", c_type_type_names[func_type->type]);

	c_type_ptr_modifier *ptr_mod;
	c_type_simple_modifier *simple_mod;
	identifier *new_identifier = malloc(sizeof(identifier));
	new_identifier->name = strdup(target->func_definition.name);
	new_identifier->type = func_type;

	while(func_type != NULL) {
		bool signed_set = false;
		bool length_set = false;
		int i;
		switch (func_type->type) {
			case C_SIMPLE:
				if ((int) func_type->simple.type < c_type_simple_names_len)
					type_name = c_type_simple_names[func_type->simple.type];
				else
					raise_err("UNKNOWN function type: %d\n", func_type->simple.type);

				RARRAY_FOREACH(simple_mod, func_type->simple.modifiers, i) {
					switch (*simple_mod) {
						case NONE:
							// UNREACHABLE
							break;
						case SIGNED:
							if (signed_set)
								raise_err("Func definition cannot specify multiple signed states, pick one: (unsigned, signed)");
							signed_set = true;
							string_builder_add_s(mod_section, "signed ");
							break;
						case UNSIGNED:
							if (signed_set)
								raise_err("Func definition cannot specify multiple signed states, pick one: (unsigned, signed)");
							signed_set = true;
							string_builder_add_s(mod_section, "unsigned ");
							break;
						case SHORT:
							if (length_set)
								raise_err("Func definition cannot specify multiple lengths, pick one: (long, short)");
							length_set = true;
							string_builder_add_s(mod_section, "short ");
							break;
						case LONG:
							if (length_set)
								raise_err("Func definition cannot specify multiple lengths, pick one: (long, short)");
							length_set = true;
							string_builder_add_s(mod_section, "long ");
							break;
						case MUTABLE:
							if (new_identifier->mutable == IDENTIFIER_UNKNOWN_MUTABILITY)
								new_identifier->mutable = IDENTIFIER_MUTABLE;
							else
								raise_err("Func definition cannot repeat mutability specifiers");
					}
				}

				if (new_identifier->mutable == IDENTIFIER_UNKNOWN_MUTABILITY)
					new_identifier->mutable = IDENTIFIER_IMMUTABLE;

				result_write("%s %s %s%s", mod_section->string, type_name, start_section->string, target->func_definition.name);
				prototype_write("%s %s %s%s", mod_section->string, type_name, start_section->string, target->func_definition.name);
				func_type = NULL;
				break;
			case C_PTR:
				new_identifier->location = IDENTIFIER_UNKNOWN;
				new_identifier->freed = false;

				RARRAY_FOREACH(ptr_mod, func_type->ptr.modifiers, i) {
					switch (*ptr_mod) {
						case STACK:
							if (new_identifier->location == IDENTIFIER_UNKNOWN)
								new_identifier->location = IDENTIFIER_STACK_ALLOCATED;
							else
								raise_err("Func definition cannot repeat allocation specifiers, pick one: (heap, stack)");
							break;
						case HEAP:
							if (new_identifier->location == IDENTIFIER_UNKNOWN)
								new_identifier->location = IDENTIFIER_HEAP_ALLOCATED;
							else
								raise_err("Func definition cannot repeat allocation specifiers, pick one: (heap, stack)");
							break;
						case IN:
						case INOUT:
						case OUT:
							raise_err("Function cannot be specified as in, out, or inout.");
							break;
					}
				}
				if (new_identifier->location == IDENTIFIER_UNKNOWN)
					printf("WARN - function is returning a pointer to an assumed stack address : %s\n", target->func_definition.name);
				string_builder_add_c(start_section, '*');
				func_type = func_type->ptr.to;
				break;
			case C_ADR:
				raise_err("Cannot have the address operator '&' in a function return type.");
			case C_ARR:
				raise_err("Cannot return an array from a function, use a pointer instead.");
			default:
				raise_err("WITH WRITING TYPE: %s\n", c_type_type_names[func_type->type]);
				break;
		}
	}
	add_to_scope(new_identifier);
	result_write("(");
	prototype_write("(");

	int i;
	fn_arg *arg;
	RARRAY_FOREACH(arg, target->func_definition.type->fn.args, i) {
		write_variable(arg->type, (char *) arg->name, true, false);
		if (i < target->func_definition.type->fn.arg_count - 1) {
			result_write(",");
			prototype_write(",");
		}
	}
	result_write(")");
	prototype_write(");\n");

	string_builder_free(&start_section);
	string_builder_free(&mod_section);
}

int write_tkn_line(tkn_line *line, char *name, bool write_newline);
int write_tkn_line(tkn_line *line, char *name, bool write_newline) {
	int i;
	tkn_line *writing_line;
	// printf("WRITING (%s) \n", line_names[line->type]);
	switch (line->type) {
		case MACRO:
			result_write("%s\n", line->macro.content);
			break;
		case VAR_DEFINITION:
			write_variable(line->var_definition.type, line->var_definition.name, false, false);
			bool var_is_stack = false;
			bool var_is_heap = false;
			c_type *simple_var = (line->var_definition.type);
			// print_type(simple_var, false);
			if (simple_var->type == C_PTR && simple_var->ptr.modifiers && simple_var->ptr.modifiers->size > 0) {
				RARRAY_CONTAINS(
					line->var_definition.type->ptr.modifiers,
					STACK,
					c_type_ptr_modifier,
					var_is_stack
				);
				RARRAY_CONTAINS(
					line->var_definition.type->ptr.modifiers,
					HEAP,
					c_type_ptr_modifier,
					var_is_heap
				);
				if (var_is_stack && var_is_heap)
					raise_err("Pointer cannot be defined as both heap and stack.");
			}

			tkn *var_tkn;
			tkn_line *var_tkns;
			if (line->var_definition.simple) {
				// TODO - exchange this logic here to use "write_statement" instead
				result_write(" = \n");
				RARRAY_FOREACH(var_tkn, line->var_definition.val, i) {
					write_tkn(var_tkn);
					if (var_tkn->type == IDENTIFIER) {
						identifier *var_identifier = scope_find(var_tkn->content);
						if (var_identifier) {
							if (var_identifier->type->type == C_FN) {
								if (i < line->var_definition.val->size - 1) {
									tkn *next_tkn = line->var_definition.val->items[++i];
									if (next_tkn->type == CONTAINER) {
										if (!compare_function_args(var_identifier->type->fn.args, next_tkn))
											raise_err("Function called with incorrect arguments: '%s'", var_tkn->content);
									}
									else raise_err("Unexpected function identifier with no arguments: '%s'", var_tkn->content);
								}
								else raise_err("Unexpected function identifier with no arguments: '%s'", var_tkn->content);
							}
						}
						else {
							raise_err("Unable to find identifier '%s' in scope", var_tkn->content);
						}
					}
				}
			}
			else {
				bool array_like = true;
				if (line->var_definition.val_lines->size > 0 && ((tkn_line *) line->var_definition.val_lines->items[0])->type == STATEMENT) {
					RARRAY_FOREACH(var_tkn, ((tkn_line *) line->var_definition.val_lines->items[0])->statement.tkns, i) {
						if (i % 2 != 0 && !(var_tkn->type == OPERATOR && strcmp(var_tkn->content, ",") == 0)) // check if every second item is a ','
							array_like = false;
					}
				}
				else array_like = false;
				if (array_like) {
					result_write(" = {");
					RARRAY_FOREACH(var_tkns, line->var_definition.val_lines, i)
						write_tkn_line(var_tkns, NULL, false);
					result_write("}");
				}
				else {
					result_write("\n");
					RARRAY_FOREACH(var_tkns, line->var_definition.val_lines, i)
						write_tkn_line(var_tkns, line->var_definition.name, true);
				}
			}
			if (write_newline)
				result_write("\n");
			break;
		case FUNC_DEFINITION:
			// write_type(*line->var_definition.type, false);
			// result_write("FUNC: %s", line->func_definition.name);
			write_function(line);
			result_write("{\n");
			create_new_scope();
			RARRAY_FOREACH(writing_line, line->func_definition.tkn_lines, i)
				write_tkn_line(writing_line, name, true);
			result_write("}\n");
			exit_scope();
			break;
		case BLOCK:
			write_block(line);
			result_write("{\n");
			create_new_scope();
			RARRAY_FOREACH(writing_line, line->block.tkn_lines, i)
				write_tkn_line(writing_line, name, true);
			result_write("}\n");
			exit_scope();
			break;
		case STATEMENT:
			if (name == NULL) {
				write_statement(line);
				if (write_newline)
					result_write("\n");
			}
			else {
				result_write("%s = ", name);
				write_statement(line);
				if (write_newline)
					result_write("\n");
			}
			break;
		case PROTOTYPE:
			raise_err("Prototypes are not supported in Shep. All functions have prototypes autogenerated and placed at the top of the resulting source code.\n");
		default:
			result_write("ELSE\n");
			break;
			// printf(" } => \n");
			// if (line->var_definition.simple)
			// 	RARRAY_FOREACH(elem, line->var_definition.val, i) {
			// 		print_tkn(elem, depth + 1);
			// 	}
			// else {
			// 	RARRAY_FOREACH(elem_line, line->var_definition.val_lines, i) {
			// 		print_tokenised_line(elem_line, depth + 1);
			// 	}
			// }
	}

	return 0;
}

int parse(rarray *tkn_lines) {
	outputFile = fopen("./out.c", "w");
	prototypeFile = fopen("./outPrototypes.h", "w");
	outputFile = freopen("./out.c", "a", outputFile);
	prototypeFile = freopen("./outPrototypes.h", "a", prototypeFile);
	result_write("#include \"outPrototypes.h\"\n");
	init_identifier_scopes();

	int i;
	tkn_line *line;
	RARRAY_FOREACH(line, tkn_lines, i) {
		write_tkn_line(line, NULL, true);
	}
	exit_scope();

	printf("Finished writing.");
	return 0;
}

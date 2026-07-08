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

FILE *prototypeFile = NULL;
FILE *outputFile = NULL;

#define prototype_write(...) { \
	if (prototypeFile != NULL) \
		fprintf(prototypeFile, __VA_ARGS__); \
}

#define result_write(...) { \
	if (outputFile != NULL) \
		fprintf(outputFile, __VA_ARGS__); \
}

// void write_type(c_type target, bool newline);
// void write_type(c_type target, bool newline) {
// 	if (target.type == C_SIMPLE) {
// 		switch (target.simple.type) {
// 			case (INT):
// 				result_write("int ");
// 				break;
// 			case (FLOAT):
// 				result_write("float ");
// 				break;
// 			case (DOUBLE):
// 				result_write("double ");
// 				break;
// 			case (CHAR):
// 				result_write("char ");
// 				break;
// 			case (BOOL):
// 				result_write("bool ");
// 				break;
// 			case (VOID):
// 				result_write("void ");
// 				break;
// 		}
// 	}
// 	else if (target.type == C_PTR) {
// 		result_write("* ");
// 		write_type(*target.ptr.to, false);
// 	}
// 	else if (target.type == C_ADR) {
// 		result_write("& ");
// 		write_type(*target.adr.to, false);
// 	}
// 	else if (target.type == C_ARR) {
// 		result_write("[%d] ", target.arr.size);
// 		write_type(*target.arr.of, false);
// 	}
// 	else if (target.type == C_ENM) {
// 		result_write("enum { ");
// 		int f_elem = target.enm.member_count - 1;
// 		for (int i = 0; i < f_elem; i++)
// 			result_write(" %s = %d, ", ((enm_member*)target.enm.members->items[i])->key, ((enm_member*)target.enm.members->items[i])->val);
// 		result_write(" %s = %d } ", ((enm_member*)target.enm.members->items[f_elem])->key, ((enm_member*)target.enm.members->items[f_elem])->val);
// 		
// 	}
// 	else if (target.type == C_STRT) {
// 		result_write("struct {");
// 		for (int i = 0; i < target.strt.member_count; i++) {
// 			write_type(*((strt_member*)target.strt.members->items[i])->val, false);
// 			result_write(" %s;", ((strt_member*)target.strt.members->items[i])->key);
// 		}
// 		result_write("}");
// 	}
// 	else if (target.type == C_UNN) {
// 		result_write("union {");
// 		for (int i = 0; i < target.unn.member_count; i++) {
// 			write_type(*((strt_member*)target.unn.members->items[i])->val, false);
// 			result_write(" %s;", ((strt_member*)target.unn.members->items[i])->key);
// 		}
// 		result_write("}");
// 	}
// 	else if (target.type == C_FN) {
// 		write_type(*target.fn.type, false);
// 		result_write("(");
// 		for (int i = 0; i < target.fn.arg_count; i++) {
// 			write_type(*((fn_arg*)target.fn.args->items[i])->type, false);
// 			result_write(",");
// 		}
// 		result_write(")");
// 	}
// 	else printf("UNKNOWN DATA TYPE... (%d)", target.type);
//
// 	if (newline) printf("\n");
// }


void write_variable(c_type *var_type, char *name, bool proto, bool type_only) {
	string_builder *closing_section = string_builder_create(10);
	string_builder *start_section = string_builder_create(10);
	string_builder *mod_section = string_builder_create(10);

	while(var_type != NULL) {
		switch (var_type->type) {
			case C_SIMPLE:
				str_rev(closing_section->string);
				char *type_name;
				if ((int) var_type->simple.type < c_type_simple_names_len)
					type_name = c_type_simple_names[var_type->simple.type];
				else {
					fprintf(stderr, "UNKNOWN var type: %d\n", var_type->simple.type);
					exit(1);
				}

				identifier *new_identifier = malloc(sizeof(identifier));
				new_identifier->mutable = IDENTIFIER_UNKNOWN_MUTABILITY;
				new_identifier->location = IDENTIFIER_UNKNOWN;

				bool signed_set = false;
				bool length_set = false;
				int i;
				c_type_simple_modifier *mod;
				RARRAY_FOREACH(mod, var_type->simple.modifiers, i) {
					switch (*mod) {
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
						case STACK:
							if (new_identifier->mutable == IDENTIFIER_UNKNOWN)
								new_identifier->mutable = IDENTIFIER_STACK_ALLOCATED;
							else
								raise_err("Var definition cannot repeat allocation specifiers, pick one: (heap, stack)");
							break;
						case HEAP:
							if (new_identifier->mutable == IDENTIFIER_UNKNOWN)
								new_identifier->mutable = IDENTIFIER_HEAP_ALLOCATED;
							else
								raise_err("Var definition cannot repeat allocation specifiers, pick one: (heap, stack)");
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

				new_identifier->name = strdup(name);
				new_identifier->type = IDENTIFIER_VAR;
				add_to_scope(new_identifier);
				break;
			case C_PTR:
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

				if (var_type->arr.size > 0) {
					char *intstr = int_to_str(var_type->arr.size);
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
				printf("ERROR WITH WRITING TYPE: %s\n", c_type_type_names[var_type->type]);
				exit(1);
				break;
			// case C_ENM:
			// 	printf("C_ENM: ");
			// 	for (int i = 0; i < var_type->enm.member_count; i++)
			// 		printf(" { %s: %d }", ((enm_member*)var_type->enm.members->items[i])->key, ((enm_member*)var_type->enm.members->items[i])->val);
			// 	break;
			// case C_STRT:
			// 	printf("C_STRT: ");
			// 	for (int i = 0; i < var_type->strt.member_count; i++) {
			// 		printf(" { %s: (", ((strt_member*)var_type->strt.members->items[i])->key);
			// 		print_type(*((strt_member*)var_type->strt.members->items[i])->val, false);
			// 		printf(") }");
			// 	}
			// case C_UNN:
			// 	printf("C_UNN: ");
			// 	for (int i = 0; i < var_type->unn.member_count; i++) {
			// 		printf(" { %s: (", ((strt_member*)var_type->unn.members->items[i])->key);
			// 		print_type(*((strt_member*)var_type->unn.members->items[i])->val, false);
			// 		printf(") }");
			// 	}
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
}

void write_tkn(tkn *target);
void write_tkn(tkn *target) {
	if (target->type == CONTAINER || target->type == OPERATOR_SLICE) {
		result_write("%s", target->type == CONTAINER ? "(" : "[");
		int i;
		tkn *sub_tkn;
		RARRAY_FOREACH(sub_tkn, target->tkn_content, i)
			write_tkn(sub_tkn);
		result_write("%s", target->type == CONTAINER ? ")" : "]");
	}
	else if (target->type == BLOCK_START) {
		result_write("{");
	}
	else if (target->type == BLOCK_END) {
		result_write("}");
	}
	else {
		result_write("%s", target->content);
	}
}

void write_statement(tkn_line *target) {
	if (target != NULL) {
		tkn *elem;
		int i;
		RARRAY_FOREACH(elem, target->statement.tkns, i) {
			if (elem->type == OPERATOR_ASSIGN) {
				if (i > 0) {
					tkn *last_tkn = target->statement.tkns->items[i-1];
					if (last_tkn->type != IDENTIFIER)
						raise_err("Assignment operators require an identifier on the left side of the operator.");
					else {
						identifier *found = scope_find(last_tkn->content, IDENTIFIER_VAR);
						if (found == NULL) {
							fprintf(stderr, "%s : ", last_tkn->content);
							raise_err("Failed to find variable identifier");
						}
						else {
							if (found->mutable != IDENTIFIER_MUTABLE) {
								fprintf(stderr, "%s : ", found->name);
								raise_err("Reassigning immutable variable. Mark variable with 'mutable' to be able to change it.");
							}
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
			result_write("(");
			write_tkn(target->block.content);
			result_write(")");
			break;
		case IDENTIFIER_ELSEIF:
			result_write("else if");
			result_write("(");
			write_tkn(target->block.content);
			result_write(")");
			break;
		case IDENTIFIER_ELSE:
			result_write("else");
			break;
		case IDENTIFIER_FOR:
			result_write("for");
			result_write("(");
			write_tkn(target->block.content);
			result_write(")");
			break;
		case IDENTIFIER_WHILE:
			result_write("while");
			result_write("(");
			write_tkn(target->block.content);
			result_write(")");
			break;
		default:
			fprintf(stderr, "Currently unsupport block type: %s\n", tkn_names[target->block.type->type]);
			exit(1);
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
		raise_err("incomplete function signature does not contain arguments.");

	while(func_type != NULL) {
		switch (func_type->type) {
			case C_SIMPLE:
				if ((int) func_type->simple.type < c_type_simple_names_len)
					type_name = c_type_simple_names[func_type->simple.type];
				else {
					fprintf(stderr, "UNKNOWN function type: %d\n", func_type->simple.type);
					exit(1);
				}
				identifier *new_identifier = malloc(sizeof(identifier));
				new_identifier->mutable = IDENTIFIER_UNKNOWN_MUTABILITY;
				new_identifier->location = IDENTIFIER_UNKNOWN;

				bool signed_set = false;
				bool length_set = false;
				int i;
				c_type_simple_modifier *mod;
				RARRAY_FOREACH(mod, func_type->simple.modifiers, i) {
					switch (*mod) {
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
						case STACK:
							if (new_identifier->mutable == IDENTIFIER_UNKNOWN)
								new_identifier->mutable = IDENTIFIER_STACK_ALLOCATED;
							else
								raise_err("Func definition cannot repeat allocation specifiers, pick one: (heap, stack)");
							break;
						case HEAP:
							if (new_identifier->mutable == IDENTIFIER_UNKNOWN)
								new_identifier->mutable = IDENTIFIER_HEAP_ALLOCATED;
							else
								raise_err("Func definition cannot repeat allocation specifiers, pick one: (heap, stack)");
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

				new_identifier->name = strdup(target->func_definition.name);
				new_identifier->type = IDENTIFIER_FUNC;
				add_to_scope(new_identifier);

				result_write("%s %s %s%s", mod_section->string, type_name, start_section->string, target->func_definition.name);
				prototype_write("%s %s %s%s", mod_section->string, type_name, start_section->string, target->func_definition.name);
				func_type = NULL;
				break;
			case C_PTR:
				string_builder_add_c(start_section, '*');
				func_type = func_type->ptr.to;
				break;
			case C_ADR:
				raise_err("Cannot have the address operator '&' in a function return type.");
			case C_ARR:
				raise_err("Cannot return an array from a function, use a pointer instead.");
			default:
				printf("ERROR WITH WRITING TYPE: %s\n", c_type_type_names[func_type->type]);
				exit(1);
				break;
		}
	}
	result_write("(");
	prototype_write("(");

	int i;
	fn_arg *arg;
	RARRAY_FOREACH(arg, target->func_definition.type->fn.args, i) {
		write_variable(arg->type, arg->name, true, false);
		if (i < target->func_definition.type->fn.arg_count - 1) {
			result_write(",");
			prototype_write(",");
		}
	}
	result_write(")");
	prototype_write(");\n");
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
			print_type(*simple_var, false);
			if (simple_var->type == C_SIMPLE && simple_var->simple.modifiers && simple_var->simple.modifiers->size > 0) {
				RARRAY_CONTAINS(
					line->var_definition.type->simple.modifiers, 
					STACK, 
					c_type_simple_modifier, 
					var_is_stack
				);
				RARRAY_CONTAINS(
					line->var_definition.type->simple.modifiers, 
					HEAP, 
					c_type_simple_modifier, 
					var_is_heap
				);
				if (var_is_stack && var_is_heap) 
					raise_err("Vars cannot be defined as both heap and stack.");
			}

			tkn *var_tkn;
			tkn_line *var_tkns;
			if (line->var_definition.simple) {
				result_write(" = \n");
				if (var_is_heap) {
					result_write(" = \n");
					c_type *var_type = line->var_definition.type;
					while(var_type != NULL) {
						switch (var_type->type) {
							case C_SIMPLE:
								break;
							case C_PTR:
								break;
							case C_ARR:
								break;
							case C_ADR:
								break;
							case C_ENM:
							case C_STRT:
							case C_UNN:
							case C_FN:
								printf(">>>>>> TODO\n\n\n");
								break;
						}
					}
				}
				else {
					RARRAY_FOREACH(var_tkn, line->var_definition.val, i)
						write_tkn(var_tkn);
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
					result_write(";\n");
					RARRAY_FOREACH(var_tkns, line->var_definition.val_lines, i)
						write_tkn_line(var_tkns, line->var_definition.name, true);
				}
			}
			if (write_newline)
				result_write(";\n");
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
					result_write(";\n");
			}
			else {
				result_write("%s = ", name);
				write_statement(line);
				if (write_newline)
					result_write(";\n");
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

	return 0;
}

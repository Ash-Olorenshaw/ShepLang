#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./string_utils.h"
#include "./utils/arrays.h"
#include "./utils/strings/type.h"
#include "./tokeniser.h"
#include "./utils/core.h"
#include "./vars.h"

#define MAX_BLOCK_DEPTH 100

char *tkn_names[] = {
	"LITERAL_NUMBER",
	"LITERAL_CHAR",
	"LITERAL_STRING",
	"IDENTIFIER",
	"IDENTIFIER_IF",
	"IDENTIFIER_ELSE",
	"IDENTIFIER_ELSEIF",
	"IDENTIFIER_WHILE",
	"IDENTIFIER_FOR",
	"IDENTIFIER_RETURN",
	"IDENTIFIER_BREAK",
	"IDENTIFIER_CONTINUE",
	"IDENTIFIER_SWITCH",
	"IDENTIFIER_CASE",
	"IDENTIFIER_DEFAULT",
	"IDENTIFIER_GOTO",
	"IDENTIFIER_SIZEOF",
	"OPERATOR",
	"OPERATOR_ASSIGN",
	"OPERATOR_SLICE",
	"CONTAINER",
	"BLOCK_START",
	"BLOCK_END"
};

char *line_names[] = {
	"VAR_DEFINITION", // TODO: typedef
	"FUNC_DEFINITION",
	"BLOCK",
	"MACRO",
	"STATEMENT", // break, return, jmp, also maybe control flow? continue, goto, etc?
	"PROTOTYPE",
	"LABEL",
};

tkn_line *create_tknline(tkn_type type, char *content) {
	tkn_line *new_tkn = malloc(sizeof(tkn_line));
	new_tkn->type = type;
	new_tkn->macro.content = content;
	return new_tkn;
}

bool is_c_macro(const char *line) {
	return (
		starts_with(line, "#include") ||
		starts_with(line, "#if") ||
		starts_with(line, "#ifdef") ||
		starts_with(line, "#ifndef") ||
		starts_with(line, "#elif") ||
		starts_with(line, "#else") ||
		starts_with(line, "#endif") ||

		starts_with(line, "#define") ||
		starts_with(line, "#undef") ||

		starts_with(line, "#pragma") ||
		starts_with(line, "#line") ||
		starts_with(line, "#error") ||
		starts_with(line, "#warning")
	);
}

void print_tkn(tkn *elem, int depth) {
	for (int i = 0; i < depth; i++)
		printf("\t");
	if (elem->type == CONTAINER || elem->type == OPERATOR_SLICE) {
		printf("%s :\n", tkn_names[elem->type]);
		int j;
		tkn *sub_elem;
		RARRAY_FOREACH(sub_elem, elem->tkn_content, j) {
			print_tkn(sub_elem, depth + 1);
		}
	}
	else if (elem->type == BLOCK_START || elem->type == BLOCK_END)
		printf("%s\n", tkn_names[elem->type]);
	else
		printf("%s : %s\n", tkn_names[elem->type], elem->content);
}

rarray *raw_tokenise_line(const char *line, bool print);
rarray *raw_tokenise_line(const char *line, bool print) {
	printf("TOKING: %s\n", line);
	rarray *line_tkns = rarray_create(10, sizeof(tkn *));

	if (line == NULL)
		return line_tkns;

	int line_len = strlen(line);
	int prev_type = CHAR_UNKNOWN, current_type = CHAR_UNKNOWN;
	char c,seeking = '\0';
	string_builder *selection = string_builder_create(1024);
	int depth = 0;

	for (int i = 0; i < line_len; i++) {
		c = line[i];
		// printf("TESTING CHAR: '%c' (%s)\n", c, selection->string);
		current_type = char_type(c);
		if (seeking != '\0') {
			string_builder_raw_add_c(selection, c);
			if ((seeking == ')' && c == '(') || (seeking == ']' && c == '['))
				depth++;
			else if ((seeking == ')' && c == ')') || (seeking == ']' && c == ']') || (seeking == '"' && c == '"') || (seeking == '\'' && c == '\'')) {
				if (depth == 0) {
					string_builder_raw_add_c(selection, '\0');
					string_builder_soft_reset(selection);
					seeking = '\0';
					tkn *new_tkn = malloc(sizeof(tkn));
					if (c == '"')
						new_tkn->type = LITERAL_STRING;
					else if (c == '\'')
						new_tkn->type = LITERAL_CHAR;
					else if (c == ')')
						new_tkn->type = CONTAINER;
					else
						new_tkn->type = OPERATOR_SLICE;

					if (c == '"' || c == '\'')
						new_tkn->content = strdup(selection->string);
					else
						new_tkn->tkn_content = raw_tokenise_line(substr(selection->string, 1, strlen(selection->string) - 1), false);
					// printf("ADDING CONTAINER %s\n", selection->string);
					rarray_add(line_tkns, new_tkn);
					// string_builder_raw_add_c(selection, c);
					string_builder_soft_reset(selection);
				}
				else
					depth--;
			}
		}
		else if ((i != 0 && current_type == prev_type) || ((c == '_' || current_type == CHAR_NUM) && prev_type == CHAR_ALPHA) || (i == 0 && current_type != CHAR_UNKNOWN)) { // TODO - all number variants: 0xFF 0.1 42 42u 42L 42UL
			if (i == 0)
				prev_type = current_type;
			string_builder_raw_add_c(selection, c);
		}
		else {
parse:
			string_builder_raw_add_c(selection, '\0');
			string_builder_soft_reset(selection);
			// printf("ELSE: %c -> %s\n", c, selection->string);
			if (c == '[' || c == '(' || c == '"' || c == '\'')
				seeking = c == '[' ? ']' : c == '(' ? ')' : c;

			if (prev_type == CHAR_SYMBOL && strlen(selection->string) > 0) {
				tkn *new_tkn = malloc(sizeof(tkn));
				new_tkn->content = strdup(selection->string);
				if (
						strchr(selection->string, '=') != NULL && 
						strcmp(selection->string, "==") != 0 && 
						strcmp(selection->string, "!=") != 0 && 
						strcmp(selection->string, ">=") != 0 && 
						strcmp(selection->string, "<=") != 0
					)
					new_tkn->type = OPERATOR_ASSIGN;
				else if (selection->string[0] == '{')
					new_tkn->type = BLOCK_START;
				else if (selection->string[0] == '}')
					new_tkn->type = BLOCK_END;
				else
					new_tkn->type = OPERATOR;
				rarray_add(line_tkns, new_tkn);
				// printf("ADDING (SYMBOL): %s\n", new_tkn->content);
			}
			else if (prev_type == CHAR_NUM) {
				tkn *new_tkn = malloc(sizeof(tkn));
				new_tkn->type = LITERAL_NUMBER;
				new_tkn->content = strdup(selection->string);
				rarray_add(line_tkns, new_tkn);
				// printf("ADDING (NUM): %s\n", new_tkn->content);
				string_builder_raw_add_c(selection, c);
				string_builder_soft_reset(selection);
			}
			else if (prev_type == CHAR_ALPHA) {
				tkn *new_tkn = malloc(sizeof(tkn));
				new_tkn->content = strdup(selection->string);

				if (strcmp(selection->string, "if") == 0)
					new_tkn->type = IDENTIFIER_IF;
				else if (strcmp(selection->string, "else") == 0)
					new_tkn->type = IDENTIFIER_ELSE;
				else if (strcmp(selection->string, "while") == 0)
					new_tkn->type = IDENTIFIER_WHILE;
				else if (strcmp(selection->string, "for") == 0)
					new_tkn->type = IDENTIFIER_FOR;
				else if (strcmp(selection->string, "return") == 0)
					new_tkn->type = IDENTIFIER_RETURN;
				else if (strcmp(selection->string, "break") == 0)
					new_tkn->type = IDENTIFIER_BREAK;
				else if (strcmp(selection->string, "continue") == 0)
					new_tkn->type = IDENTIFIER_CONTINUE;
				else if (strcmp(selection->string, "switch") == 0)
					new_tkn->type = IDENTIFIER_SWITCH;
				else if (strcmp(selection->string, "case") == 0)
					new_tkn->type = IDENTIFIER_CASE;
				else if (strcmp(selection->string, "default") == 0)
					new_tkn->type = IDENTIFIER_DEFAULT;
				else if (strcmp(selection->string, "goto") == 0)
					new_tkn->type = IDENTIFIER_GOTO;
				else if (strcmp(selection->string, "sizeof") == 0)
					new_tkn->type = IDENTIFIER_SIZEOF;
				else
					new_tkn->type = IDENTIFIER;

				rarray_add(line_tkns, new_tkn);
				// printf("ADDING (IDENTIFIER): %s\n", new_tkn->content);
			}

			if (c != '\0') {
				string_builder_raw_add_c(selection, c);
				prev_type = current_type;
			}
		}
	}

	if (selection->size > 0) {
		// printf("SELECTION: %s %d\n", selection->string, selection->size);
		c = '\0';
		goto parse;
	}

	if (print) {
		printf("\nLINE '%s' vvv\n", line);

		int i;
		tkn *elem;
		RARRAY_FOREACH(elem, line_tkns, i) {
			print_tkn(elem, 0);
		}

		printf("LINE END ^^^\n");
	}
	return line_tkns;
}

rarray **tokenise_lines(rarray *lines) {
	char *line;
	int i;
	rarray **parent_tkns = malloc(sizeof(rarray) * MAX_BLOCK_DEPTH);
	tkn_line *new_tkn = malloc(sizeof(tkn_line));
	bool new_tkn_created = false;
	parent_tkns[0] = rarray_create(100, sizeof(tkn_line));
	int parent_tkns_pos = 0;
	
	RARRAY_FOREACH(line, lines, i) {
		if (is_c_macro(line)) {
			new_tkn->type = MACRO;
			new_tkn->macro.content = line;
			new_tkn_created = true;
		}
		else if (line[strlen(line) - 1] == ':') {
			new_tkn->type = LABEL;
			new_tkn->label.name = substr(line, 0, strlen(line) - 1);
			new_tkn_created = true;
		}
		else {
			remove_unnecessary_whitespace(line);
			rarray *tkns = raw_tokenise_line(line, true);

			if (tkns->size < 1) 
				continue;
			tkn *tkn_1 = tkns->items[0];
			tkn *tkn_last = tkns->items[tkns->size - 1];

			if (tkn_1->type == IDENTIFIER && (
					strcmp(tkn_1->content, "const") == 0 ||
					strcmp(tkn_1->content, "struct") == 0 ||
					strcmp(tkn_1->content, "union") == 0 ||
					strcmp(tkn_1->content, "enum") == 0 ||
					strcmp(tkn_1->content, "signed") == 0 ||
					strcmp(tkn_1->content, "unsigned") == 0 ||
					strcmp(tkn_1->content, "short") == 0 ||
					strcmp(tkn_1->content, "long") == 0 ||
					strcmp(tkn_1->content, "int") == 0 ||
					strcmp(tkn_1->content, "float") == 0 ||
					strcmp(tkn_1->content, "double") == 0 ||
					strcmp(tkn_1->content, "char") == 0 ||
					strcmp(tkn_1->content, "bool") == 0
				))
			{
				int i, assignments = 0, assignment_pos = -1;
				tkn *target;
				RARRAY_FOREACH(target, tkns, i) {
					if (target->type == OPERATOR_ASSIGN) {
						assignments += 1;
						if (assignments == 1)
							assignment_pos = i;
					}
				}

				if (assignments > 1)
					raise_err("Cannot have multiple top-level variable assignments");
				else if (assignments == 1) {
					new_tkn->type = VAR_DEFINITION;
					new_tkn->var_definition.type = get_var_info(substr(line, 0, strchr(line, '=') - line), &(new_tkn->var_definition.name));
					if (tkn_last->type == BLOCK_START) {
						new_tkn->var_definition.simple = false;
						new_tkn->var_definition.val_lines = rarray_create(50, sizeof(tkn_line));
						
						// NOTE: manually add new tkn instead of using 'new_tkn_created' since using it will create a circular ref.
						rarray_add(parent_tkns[parent_tkns_pos], new_tkn);
						if (parent_tkns_pos < MAX_BLOCK_DEPTH - 1)
							parent_tkns[++parent_tkns_pos] = new_tkn->var_definition.val_lines;
						else
							raise_err("Reached max nested block limit of 100.");
					}
					else {
						new_tkn->var_definition.simple = true;
						assignment_pos++;
						new_tkn->var_definition.val = rarray_slice(tkns, assignment_pos, tkns->size - assignment_pos);
						new_tkn_created = true;
					}
				}
				else {
					if (tkn_last->type == BLOCK_START) {
						new_tkn->type = FUNC_DEFINITION;
						new_tkn->func_definition.tkn_lines = rarray_create(50, sizeof(tkn_line));
						new_tkn->func_definition.type = get_var_info(line, &(new_tkn->func_definition.name));
						
						// NOTE: manually add new tkn instead of using 'new_tkn_created' since using it will create a circular ref.
						rarray_add(parent_tkns[parent_tkns_pos], new_tkn);
						if (parent_tkns_pos < MAX_BLOCK_DEPTH - 1)
							parent_tkns[++parent_tkns_pos] = new_tkn->func_definition.tkn_lines;
						else
							raise_err("Reached max nested block limit of 100.");
					}
					else {
						new_tkn->type = PROTOTYPE;
						new_tkn->prototype.type = get_var_info(line, &(new_tkn->prototype.name));
						new_tkn_created = true;
					}
				}
			}
			else if (
				tkn_1->type == IDENTIFIER_ELSE ||
				tkn_1->type == IDENTIFIER_IF ||
				tkn_1->type == IDENTIFIER_WHILE ||
				tkn_1->type == IDENTIFIER_FOR ||
				tkn_1->type == IDENTIFIER_SWITCH
			) {
				// TODO - else if 
				new_tkn->type = BLOCK;
				new_tkn->block.type = tkn_1;

				int content_start = 2;

				if (tkn_1->type != IDENTIFIER_ELSE) {
					content_start = 3;
					if (tkns->size > 2)
						new_tkn->block.content = tkns->items[1];
					else
						raise_err("Incomplete block statement (if/while/case)");
				}
				else if (tkn_1->type == IDENTIFIER_ELSE && ((tkn *)tkns->items[1])->type == IDENTIFIER_IF) {
					new_tkn->block.type->type = IDENTIFIER_ELSEIF;
					content_start = 4;
					if (tkns->size > 3)
						new_tkn->block.content = tkns->items[2];
					else
						raise_err("Incomplete block statement (else if)");
				}
				else
					new_tkn->block.content = NULL;

				if (tkn_last->type == BLOCK_START) {
					new_tkn->block.tkn_lines = rarray_create(50, sizeof(tkn_line));

					// NOTE: manually add new tkn instead of using 'new_tkn_created' since using it will create a circular ref.
					rarray_add(parent_tkns[parent_tkns_pos], new_tkn);
					if (parent_tkns_pos < MAX_BLOCK_DEPTH - 1)
						parent_tkns[++parent_tkns_pos] = new_tkn->block.tkn_lines;
					else
						raise_err("Reached max nested block limit of 100.");
				}
				else {
					tkn_line *new_line = malloc(sizeof(tkn_line));
					new_line->type = STATEMENT;
					new_line->statement.tkns = rarray_slice(tkns, content_start, tkns->size - content_start);

					new_tkn->block.tkn_lines = rarray_create(1, sizeof(tkn_line));
					rarray_add(new_tkn->block.tkn_lines, new_line);
				}
			}
			else if (tkn_1->type == BLOCK_END) {
				if (parent_tkns_pos > 0)
					parent_tkns_pos--;
				else
					raise_err("Orphaned '}'");
			}
			else {
				new_tkn->type = STATEMENT;
				new_tkn->statement.tkns = tkns;
				new_tkn_created = true;
			}
		}

		if (new_tkn_created) {
			rarray_add(parent_tkns[parent_tkns_pos], new_tkn);
			new_tkn_created = false;
		}
		new_tkn = malloc(sizeof(tkn_line));
	}

	return &parent_tkns[0];
}

void print_tokenised_line(tkn_line *line, int depth) {
	tkn *elem;
	tkn_line *elem_line;
	int i;

	for (int j = 0; j < depth; j++)
		printf("\t");

	switch (line->type) {
		case (VAR_DEFINITION):
			printf("VAR DEFINITION: { (%s) : ", line->var_definition.name);
			print_type(*line->var_definition.type, false);
			if (line->var_definition.simple) {
				printf(" (simple) } => \n");
				RARRAY_FOREACH(elem, line->var_definition.val, i) {
					print_tkn(elem, depth + 1);
				}
			}
			else {
				printf(" (complx) } => \n");
				RARRAY_FOREACH(elem_line, line->var_definition.val_lines, i) {
					print_tokenised_line(elem_line, depth + 1);
				}
			}
			break;
		case (FUNC_DEFINITION):
			printf("FUNC DEFINITION: { (%s) : ", line->func_definition.name);
			print_type(*line->func_definition.type, false);
			printf(" } => \n");
			RARRAY_FOREACH(elem_line, line->func_definition.tkn_lines, i) {
				print_tokenised_line(elem_line, depth + 1);
			}
			printf("\n");
			break;
		case (BLOCK):
			printf("BLOCK: (%s)\n", tkn_names[line->block.type->type]);
			if (line->block.content != NULL)
				print_tkn(line->block.content, depth + 1);
			RARRAY_FOREACH(elem_line, line->block.tkn_lines, i) {
				print_tokenised_line(elem_line, depth + 1);
			}
			printf("\n");
			break;
		case (MACRO):
			printf("MACRO: %s\n", line->macro.content);
			break;
		case (STATEMENT):
			printf("STATEMENT: \n");
			RARRAY_FOREACH(elem, line->statement.tkns, i) {
				print_tkn(elem, depth + 1);
			}
			printf("\n");
			break;
		case (PROTOTYPE):
			printf("PROTOTYPE:  { (%s) : ", line->prototype.name);
			print_type(*line->prototype.type, false);
			printf(" };\n");
			break;
		case (LABEL):
			printf("LABEL: %s\n", line->label.name);
			break;
		default:
			printf("UNKNOWN LINE: ");
			break;
	}
}

rarray **tokenise(rarray *file_lines) {
	rarray **new_tkn_lines = tokenise_lines(file_lines);

	printf("GOT ARRAY: %d\n", (*new_tkn_lines)->size);
	printf("MAX: %d\n", (*new_tkn_lines)->max_size);
	printf("======================================== \n\n");
	int i;
	tkn_line *line;
	RARRAY_FOREACH(line, *new_tkn_lines, i) {
		printf("\t\tSPECIFIC LINE : %d\n", i);
		if (line != NULL) {
			print_tokenised_line(line, 0); // token_lines->items[i]
		}
		else 
			printf("\nEMPTY LINE\n");
	}
	printf("======================================== \n\n");
	printf("GOT ARRAY: %d\n", (*new_tkn_lines)->size);
	printf("MAX: %d\n", (*new_tkn_lines)->max_size);

	return new_tkn_lines;
}

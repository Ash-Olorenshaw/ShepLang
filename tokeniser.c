#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "./string_utils.h"
#include "./arrays.h"
#include "./utils.h"
#include "./types.h"
#include "./tokeniser.h"

char *tkn_names[] = {
	"LITERAL_NUMBER",
	"LITERAL_CHAR",
	"LITERAL_STRING",
	"IDENTIFIER",
	"IDENTIFIER_IF",
	"IDENTIFIER_ELSE",
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
	"CONTAINER_BLOCK"
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

void print_tkn(tkn *elem) {
	if (elem->type == CONTAINER || elem->type == CONTAINER_BLOCK || elem->type == OPERATOR_SLICE) {
		printf("%s :\n", tkn_names[elem->type]);
		int j;
		tkn *sub_elem;
		RARRAY_FOREACH(sub_elem, elem->tkn_content, j) {
			print_tkn(sub_elem);
		}

	}
	else
		printf("%s : %s\n", tkn_names[elem->type], elem->content);
}

rarray *tokenise_line(const char *line, bool print);
rarray *tokenise_line(const char *line, bool print) {
	rarray *line_tkns = rarray_create(10, sizeof(tkn *));
	int line_len = strlen(line);
	int prev_type = CHAR_UNKNOWN, current_type = CHAR_UNKNOWN;
	char c, *selection = malloc(1024 * sizeof(char)), seeking = '\0';
	int selection_pos = 0, depth = 0;

	for (int i = 0; i < line_len; i++) {
		c = line[i];
		current_type = char_type(c);
		if (seeking != '\0') {
			selection[selection_pos++] = c;
			if ((seeking == ')' && c == '(') || (seeking == '}' && c == '{') || (seeking == ']' && c == '['))
				depth++;
			else if ((seeking == ')' && c == ')') || (seeking == '}' && c == '}') || (seeking == ']' && c == ']') || (seeking == '"' && c == '"') || (seeking == '\'' && c == '\'')) {
				selection[selection_pos] = '\0';
				selection_pos = 0;
				if (depth == 0) {
					seeking = '\0';
					tkn *new_tkn = malloc(sizeof(tkn));
					if (c == '"')
						new_tkn->type = LITERAL_STRING;
					else if (c == '\'')
						new_tkn->type = LITERAL_CHAR;
					else if (c == ')')
						new_tkn->type = CONTAINER;
					else if (c == '}')
						new_tkn->type = CONTAINER_BLOCK;
					else
						new_tkn->type = OPERATOR_SLICE;

					if (c == '"' || c == '\'')
						new_tkn->content = strdup(selection);
					else
						new_tkn->tkn_content = tokenise_line(selection, false);
					rarray_add(line_tkns, new_tkn);
					selection[0] = c;
				}
				else
					depth--;
			}
		}
		else if (current_type == prev_type || ((c == '_' || current_type == CHAR_NUM) && prev_type == CHAR_ALPHA) || i == 0) // TODO - all number variants: 0xFF 0.1 42 42u 42L 42UL, plus allow numbers in var names
			selection[selection_pos++] = c;
		else {
			if (c == '[' || c == '(' || c == '{' || c == '"' || c == '\'')
				seeking = c == '[' ? ']' : c == '(' ? ')' : c == '{' ? '}' : c;

			if (prev_type == CHAR_SYMBOL) {
				selection[selection_pos] = '\0';
				selection_pos = 0;
				tkn *new_tkn = malloc(sizeof(tkn));
				new_tkn->content = strdup(selection);
				if (strchr(selection, '=') != NULL && strcmp(selection, "==") != 0 && strcmp(selection, "!=") != 0 && strcmp(selection, ">=") != 0 && strcmp(selection, "<=") != 0)
					new_tkn->type = OPERATOR_ASSIGN;
				else
					new_tkn->type = OPERATOR;
				rarray_add(line_tkns, new_tkn);
				selection[0] = c;
			}
			else if (prev_type == CHAR_NUM) {
				selection[selection_pos] = '\0';
				selection_pos = 0;
				tkn *new_tkn = malloc(sizeof(tkn));
				new_tkn->type = LITERAL_NUMBER;
				new_tkn->content = strdup(selection);
				rarray_add(line_tkns, new_tkn);
				selection[0] = c;
			}
			else if (prev_type == CHAR_ALPHA) {
				selection[selection_pos] = '\0';
				selection_pos = 0;
				tkn *new_tkn = malloc(sizeof(tkn));
				new_tkn->content = strdup(selection);

				if (strcmp(selection, "if") == 0)
					new_tkn->type = IDENTIFIER_IF;
				else if (strcmp(selection, "else") == 0)
					new_tkn->type = IDENTIFIER_ELSE;
				else if (strcmp(selection, "while") == 0)
					new_tkn->type = IDENTIFIER_WHILE;
				else if (strcmp(selection, "for") == 0)
					new_tkn->type = IDENTIFIER_FOR;
				else if (strcmp(selection, "return") == 0)
					new_tkn->type = IDENTIFIER_RETURN;
				else if (strcmp(selection, "break") == 0)
					new_tkn->type = IDENTIFIER_BREAK;
				else if (strcmp(selection, "continue") == 0)
					new_tkn->type = IDENTIFIER_CONTINUE;
				else if (strcmp(selection, "switch") == 0)
					new_tkn->type = IDENTIFIER_SWITCH;
				else if (strcmp(selection, "case") == 0)
					new_tkn->type = IDENTIFIER_CASE;
				else if (strcmp(selection, "default") == 0)
					new_tkn->type = IDENTIFIER_DEFAULT;
				else if (strcmp(selection, "goto") == 0)
					new_tkn->type = IDENTIFIER_GOTO;
				else if (strcmp(selection, "sizeof") == 0)
					new_tkn->type = IDENTIFIER_SIZEOF;
				else
					new_tkn->type = IDENTIFIER;

				rarray_add(line_tkns, new_tkn);
			}


			selection[selection_pos++] = c;
			prev_type = current_type;
		}
	}

	// tkn_line *new_tkn = malloc(sizeof(tkn_line));
	// new_tkn->type = STATEMENT;
	// new_tkn->statement.tkns = line_tkns;
	// rarray_add(token_lines, new_tkn);

	if (print) {
		printf("\nLINE '%s' vvv\n", line);

		int i;
		tkn *elem;
		RARRAY_FOREACH(elem, line_tkns, i) {
			print_tkn(elem);
		}

		printf("LINE END ^^^\n");
	}
	return line_tkns;
}

int tokenise(rarray *file_lines) {
	rarray *token_lines = rarray_create(100, sizeof(tkn_line));

	int i;
	char *line;
	RARRAY_FOREACH(line, file_lines, i) {
		// if (!starts_with(remove_unnecessary_whitespace(line), "//"))
		remove_unnecessary_whitespace(line);

		if (is_c_macro(line)) {
			tkn_line *new_tkn = create_tknline(MACRO, line);
			rarray_add(token_lines, new_tkn);
		}
		else {
			tokenise_line(line, true);
		}
		// else if (is_var_def(line)) {
		// 	var_info *var = extract_var_info(line);
		// 	token *new_tkn = create_tkn(VAR_DEF, line);
		// 	new_tkn->data = (token_data){
		// 		.var_def = {
		// 			.identifier = var->var_identifier,
		// 			.type = var->var_type,
		// 			.value = var->var_type,
		// 		}
		// 	};
		// 	rarray_add(tokens, new_tkn);
		// }
	}

	return 1;
}

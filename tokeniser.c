#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "./string_utils.h"
#include "./arrays.h"
#include "./utils.h"
#include "./types.h"
#include "./tokeniser.h"

tkn *create_tkn(tkn_type type, char *content) {
	tkn *new_tkn = malloc(sizeof(tkn));
	new_tkn->type = type;
	new_tkn->content = content;
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

int tokenise(rarray *file_lines) {
	rarray *tokens = rarray_create(100, sizeof(tkn));

	int i;
	char *line;
	RARRAY_FOREACH(line, file_lines, i) {
		// if (!starts_with(remove_unnecessary_whitespace(line), "//"))
			printf("%s\n", line);
			printf("%s\n\n", remove_unnecessary_whitespace(line));
		// if (is_c_macro(line)) {
		// 	tkn *new_tkn = create_tkn(MACRO, line);
		// 	rarray_add(tokens, new_tkn);
		// }
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

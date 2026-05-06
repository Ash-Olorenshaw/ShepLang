#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "../string_utils.h"
#include "../arrays.h"
#include "./raw.h"

#define NEW_VAR_ELEM(var_elem_elem, var_elem_start) ({ \
		var_elem *res = malloc(sizeof(var_elem)); \
		res->elem = var_elem_elem; \
		res->start = var_elem_start; \
		res; \
	})

rarray *get_var_elems(const char *line) {
	int line_len, start, skip_to;
	char ch;
	rarray *raw_elems = rarray_create(10, sizeof(char*));

	line_len = strlen(line);
	ch = '\0';
	skip_to = 0, start = 0;

	for (int i = 0; i < line_len; i++) {
		ch = line[i];
		if (i < skip_to) continue;
		else if (i == skip_to && ch == ']')
			rarray_add(raw_elems, NEW_VAR_ELEM(substr(line, start, i+1), start));
		if (ch == '(') {
			if (start < i)
				rarray_add(raw_elems, NEW_VAR_ELEM(substr(line, start, i), start));
			int last = find_closing_bracket(line, i + 1, false) + 1;
			if (last < line_len && line[last] == '[') {
				int closing_bracket = seek_char(line, last, ']') + 1;
				rarray_add(raw_elems, NEW_VAR_ELEM(substr(line, last, closing_bracket), last));
			}
			start = i + 1;
		}
		else if (ch == ')' || isspace(ch)) {
			char *substring = substr(line, start, i);
			if (start < i && seek_char(substring, 0, '[') == -1)
				rarray_add(raw_elems, NEW_VAR_ELEM(substring, start));
			else free(substring);
			start = i + 1;
		}
		else if (ch == '*' || ch == '&') {
			if (start < i-1)
				rarray_add(raw_elems, NEW_VAR_ELEM(substr(line, start, i-1), start));
			rarray_add(raw_elems, NEW_VAR_ELEM(strdup((char[]){ ch, '\0' }), i));

			start = i+1;
		}
		else if (ch == '[' && isalnum(line[i-1])) {
			if (start < i-1)
				rarray_add(raw_elems, NEW_VAR_ELEM(substr(line, start, i), start));
			start = i;
			skip_to = seek_char(line, i, ']');
		}
		else if (ch == '{') {
			skip_to = seek_char(line, i, '}');
			if (skip_to > -1) {
				rarray_add(raw_elems, NEW_VAR_ELEM(substr(line, i, skip_to + 1), i));
				start = skip_to + 2;
			}
		}
		else if (i == line_len - 1) {
			char *substring = substr(line, start, i);
			rarray_add(raw_elems, NEW_VAR_ELEM(substring, start));
		}
	}

	return raw_elems;
}

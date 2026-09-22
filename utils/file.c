#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "strings/type.h"
#include "strings/utils.h"
#include "arrays.h"
#include "core.h"

#define MAX_LINE_LEN 1024

typedef enum {
	state_comment,
	state_string,
	state_container,
	state_command
} file_read_state;

rarray *get_file_lines(const char *file) {
	char ch, prev_ch, string_state;
	string_builder *current_line = string_builder_create(MAX_LINE_LEN);
	FILE *file_pointer;
	file_read_state state;
	enum {
		SEEK_NONE,
		SEEK_NEWLINE,
		SEEK_SINGLE_COMMENT_END,
		SEEK_COMMENT_END
	} seek;
	rarray *prog_lines;

	if (access(file, F_OK) != 0) {
		fprintf(stderr, "Attempted to load file \"%s\"\n", file);
		raise_err("File does not exist.");
	}

	file_pointer = fopen(file, "r");
	ch = '\0';

	string_state = '\0';
	state = state_command;
	prev_ch = '\0';
	seek = SEEK_NONE;
	prog_lines = rarray_create(128, sizeof(char *));

	while((ch = fgetc(file_pointer)) != EOF) {
		string_builder_add_c(current_line, ch);

		if (((seek == SEEK_NEWLINE || seek == SEEK_SINGLE_COMMENT_END) && ch != '\n') || (seek == SEEK_COMMENT_END && !(prev_ch == '*' && ch == '/')))
			continue;

		if (ch == '*' && prev_ch == '/')
			seek = SEEK_COMMENT_END;
		if (ch == '/' && prev_ch == '/')
			seek = SEEK_SINGLE_COMMENT_END;
		if (ch == '\n' && prev_ch != '\\' && (seek == SEEK_NEWLINE || seek == SEEK_SINGLE_COMMENT_END)) {
			if (seek == SEEK_NEWLINE && !rarray_add(prog_lines, strdup(trim(current_line->string))))
				raise_err("Failed to reallocate array when processing file line.");
			seek = SEEK_NONE;
			string_builder_reset(current_line);
		}
		else if (state == state_command) {
			if (ch == '#') {
				seek = SEEK_NEWLINE;
			}
			else if (ch == '(') {
				state = state_container;
			}
			else if ( ch == '/' && prev_ch == '/') {
				seek = SEEK_SINGLE_COMMENT_END;
			}
			else if (ch == ';' || ch == '}' || ch == '{') {
				if (ch == '{') {
					if (!rarray_add(prog_lines, strdup(trim(current_line->string))))
						raise_err("Failed to reallocate array when processing file line.");
				}
				else if (ch == '}') {
					if (strlen(trim(current_line->string)) > 1 && !rarray_add(prog_lines, substr(current_line->string, 0, strlen(current_line->string) - 1)))
						raise_err("Failed to reallocate array when processing file line.");
					if (!rarray_add(prog_lines, strdup("}")))
						raise_err("Failed to reallocate array when processing file line.");
				}
				else if (!rarray_add(prog_lines, strdup(trim(current_line->string))))
					raise_err("Failed to reallocate array when processing file line.");

				string_builder_reset(current_line);
			}
		}
		else if (ch == ')' && state == state_container) {
			state = state_command;
		}
		else if ((ch == '"' || ch == '\'') && prev_ch != '\\') {
			if (string_state == ch) {
				string_state = '\0';
				state = state_command;
			}
			else if (string_state == '\0') {
				string_state = ch;
				state = state_string;
			}
		}

		prev_ch = ch;
	}

	int i;
	char *prog_line;
	RARRAY_FOREACH(prog_line, prog_lines, i) {
		printf("LINE: %s\n", prog_line);
	}

	string_builder_free(&current_line);
	return prog_lines;
}

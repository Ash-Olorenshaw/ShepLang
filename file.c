#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "./string_utils.h"
#include "./arrays.h"
#include "./utils.h"

#define MAX_LINE_LEN 1024

typedef enum {
	state_comment,
	state_string,
	state_command
} file_read_state;

rarray *get_file_lines(const char *file) {
	int current_line_ptr;
	char ch, prev_ch, string_state, current_line[MAX_LINE_LEN];
	FILE *file_pointer;
	file_read_state state;
	enum {
		SEEK_NONE,
		SEEK_NEWLINE,
		SEEK_SINGLE_COMMENT_END,
		SEEK_COMMENT_END
	} seek;
	rarray *prog_lines;

	file_pointer = fopen(file, "r");
	current_line_ptr = 0;
	ch = '\0';

	string_state = '\0';
	state = state_command;
	prev_ch = '\0';
	seek = SEEK_NONE;
	prog_lines = rarray_create(128, sizeof(char *));

	while((ch = fgetc(file_pointer)) != EOF) {
		current_line[current_line_ptr++] = ch;
		current_line[current_line_ptr] = '\0';

		if (((seek == SEEK_NEWLINE || seek == SEEK_SINGLE_COMMENT_END) && ch != '\n') || (seek == SEEK_COMMENT_END && !(prev_ch == '*' && ch == '/')))
			continue;

		if (ch == '*' && prev_ch == '/')
			seek = SEEK_COMMENT_END;
		if (ch == '/' && prev_ch == '/')
			seek = SEEK_SINGLE_COMMENT_END;
		if (ch == '\n' && prev_ch != '\\' && (seek == SEEK_NEWLINE || seek == SEEK_SINGLE_COMMENT_END)) {
			if (seek == SEEK_NEWLINE && !rarray_add(prog_lines, strdup(trim(current_line))))
				raise_err("Failed to reallocate array when processing file line.");
			seek = SEEK_NONE;
			current_line_ptr = 0;
			current_line[current_line_ptr] = '\0';
		}
		else if (state == state_command) {
			if (ch == '#') {
				seek = SEEK_NEWLINE;
			}
			else if ( ch == '/' && prev_ch == '/') {
				seek = SEEK_SINGLE_COMMENT_END;
			}
			else if ((ch == ';' || ch == '}' || ch == '{') && state == state_command) {
				if (ch == '}' || ch == '{') {
					current_line[current_line_ptr - 1] = '\0';
					if (!rarray_add(prog_lines, strdup(trim(current_line))))
						raise_err("Failed to reallocate array when processing file line.");
					if (!rarray_add(prog_lines, strdup((char[2]){ ch, '\0' })))
						raise_err("Failed to reallocate array when processing file line.");
				}
				else if (!rarray_add(prog_lines, strdup(trim(current_line))))
					raise_err("Failed to reallocate array when processing file line.");

				current_line_ptr = 0;
				current_line[current_line_ptr] = '\0';
			}
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

	return prog_lines;
}

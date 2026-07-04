#if 0
	printf "BUILDING SCRIPT...\n"
    gcc "$0" \
		./build_scripts/utils.c ./build_scripts/args.c \
		./utils/core.c ./utils/arrays.c \
		-o ./.temp-run \
		-Wall -Wextra -pedantic
	printf "\n"
	./.temp-run "$@"
    rm -f ./.temp-run
    exit
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include "build_scripts/utils.h"
#include "build_scripts/args.h"
#include "./utils/arrays.h"
#include "./utils/core.h"

#define BUILD_FILES "tokeniser.c", "parser/parser.c", "parser/vars.c", "utils/core.c", "utils/arrays.c", "utils/file.c", "utils/strings/utils.c", "utils/strings/type.c", "vars.c", "types.c", "./vars/types.c", "./vars/compound_types.c", "./vars/raw.c"
#define OUTPUT_FILE "./shep"
#define BUILD_ARGS "-lm", "-Wall", "-Wextra", "-pedantic"
#define DEBUG_ARGS "-g", "-fsanitize=address", "-fno-omit-frame-pointer"

void build_file(char *main_file, bool debug, char *output_file) {
	rarray *build_args = rarray_create(100, sizeof(char *));
	char *build_args_c[] = { "gcc", main_file, BUILD_FILES, "-o", output_file, BUILD_ARGS, NULL };
	RARRAY_ASSIGN(build_args, build_args_c);

	if (debug) {
		char *debug_args_c[] = { DEBUG_ARGS, NULL };
		RARRAY_ASSIGN(build_args, debug_args_c);
	}

	int time = run_command((char *const *)build_args->items, ".", false);
	if (time == -1)
		exit(1);
	printf(">>> \"%s\" built in %d seconds.\n", strcmp(main_file, "main.c") == 0 ? "Program" : main_file, time);
}

int main(int argv, const char **argc) {

	args arguments = { .arg_count = argv, .args = argc };

	if (arg_pos("test", arguments) != -1) {
		struct dirent* in_file;
		DIR *tests_dir = opendir("tests");

		if (NULL == tests_dir)
			raise_err("Failed to open tests directory './tests'");
		while ((in_file = readdir(tests_dir))) {
			if (!strcmp(in_file->d_name, "."))
				continue;
			if (!strcmp(in_file->d_name, ".."))    
				continue;

			char *testfile_path = malloc(strlen("tests/") + strlen((in_file->d_name)) * sizeof(char));
			sprintf(testfile_path, "tests/%s", in_file->d_name);
			char *testfile_bin = malloc(strlen((testfile_path) + strlen(".bin")) * sizeof(char));
			sprintf(testfile_bin, "%s.bin", testfile_path);
			
			build_file(testfile_path, arg_pos("debug", arguments) != -1, testfile_bin);
			char *run_args[] = { testfile_bin, NULL };
			if (run_command(run_args, ".", false) == -1)
				exit(1);
			char *rm_args[] = { "rm", testfile_bin, NULL };
			if (run_command(rm_args, ".", false) == -1)
				exit(1);
			free(testfile_bin);
		}
	}

	build_file("main.c", arg_pos("debug", arguments) != -1, OUTPUT_FILE);

	if (arg_pos("run", arguments) != -1) {
		char *run_args[] = { OUTPUT_FILE, "./main.shep", NULL };
		if (run_command(run_args, ".", false) == -1)
			exit(1);
		char *rm_args[] = { "rm", OUTPUT_FILE, NULL };
		if (run_command(rm_args, ".", false) == -1)
			exit(1);
	}
}


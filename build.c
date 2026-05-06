#if 0
	printf "BUILDING SCRIPT...\n"
    gcc "$0" \
		./build_scripts/utils.c ./build_scripts/args.c \
		./utils.c ./arrays.c \
		-o ./.temp-run \
		-Wall -Wextra -pedantic
	printf "\n"
	./.temp-run "$@"
    rm -f ./.temp-run
    exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "build_scripts/utils.h"
#include "build_scripts/args.h"
#include "./utils.h"
#include "./arrays.h"

#define BUILD_FILES "main.c", "tokeniser.c", "string_utils.c", "utils.c", "arrays.c", "file.c", "vars.c", "types.c", "./vars/types.c", "./vars/compound_types.c", "./vars/raw.c"
#define OUTPUT_FILE "./shep"
#define BUILD_ARGS "-Wall", "-Wextra", "-pedantic"
#define DEBUG_ARGS "-g", "-fsanitize=address", "-fno-omit-frame-pointer"


int main(int argv, const char **argc) {
	args arguments = { .arg_count = argv, .args = argc };

	rarray *build_args = rarray_create(100, sizeof(char *));

	char *build_args_c[] = { "gcc", BUILD_FILES, "-o", OUTPUT_FILE, BUILD_ARGS, NULL };
	RARRAY_ASSIGN(build_args, build_args_c);

	if (arg_pos("debug", arguments) != -1) {
		char *debug_args_c[] = { DEBUG_ARGS, NULL };
		RARRAY_ASSIGN(build_args, debug_args_c);
	}

	printf("\nFINISHED ARGS\n");

	int time = run_command((char *const *)build_args->items, ".", false);
	printf("Build finished in %d seconds.", time);

	if (arg_pos("run", arguments) != -1) {
		char *run_args[] = { OUTPUT_FILE, NULL };
		run_command(run_args, ".", false);
	}
}


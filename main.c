#include <stdio.h>
#include "./utils/file.h"
// #include "vars.h"
#include "parser/parser.h"
#include "tokeniser.h"

int main(int argc, char *argv[]) {
	rarray *program_lines = get_file_lines(argc < 2 ? "main.c" : argv[1]);
	rarray **tkns = tokenise(program_lines);
	parse(*tkns);
	rarray_free(*tkns);


	// get_var_info("int (*(foo))[123]");
	// get_var_info("int (*foo)[123]");
	// get_var_info("int *(*(foo)[])");
	// get_var_info("int **foo[]");
	// get_var_info("int (*(foo))()");
	// get_var_info("int foo[123]");
	// get_var_info("int (*func_ptr)(double **foo, double **foo, double foo)");
	// get_var_info("int func_ptr(double **foo, double **foo, double foo)");
	// get_var_info("struct foo { int c; double **foobar; char *name; } foo");
	// get_var_info("struct foo foo");
	// get_var_info("union foo { int count; double **foobar; char *name; } foo");
	// get_var_info("enum foo { foo_1, foo_2, foo_3, foo_4, } foo");
}



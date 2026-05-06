#include "../vars.h"

int run() {
	// get_file_lines(argc < 2 ? "main.c" : argv[1]);

	// get_var_info("int (*(foo))[123]");
	// get_var_info("int (*foo)[123]");
	// get_var_info("int *(*(foo)[])");
	// get_var_info("int **foo[]");
	// get_var_info("int (*(foo))()");
	// get_var_info("int foo[123]");
	get_var_info("int (*func_ptr)(double **foo, double **foo, double foo)");
	get_var_info("int func_ptr(double **foo, double **foo, double foo)");
}



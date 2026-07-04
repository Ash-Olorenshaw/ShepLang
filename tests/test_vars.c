#include <stdio.h>
#include "../vars.h"

int main(void) {
	// get_file_lines(argc < 2 ? "main.c" : argv[1]);

	// get_var_info("int (*(foo))[123]");
	// get_var_info("int (*foo)[123]");
	// get_var_info("int *(*(foo)[])");
	// get_var_info("int **foo[]");
	// get_var_info("int (*(foo))()");
	// get_var_info("int foo[123]");
	char *name;
	char *test;

	test = "int (*func_ptr)(double **foo, double **foo, double foo)";
	printf("Generating var info: %s\n", test);
	get_var_info(test, &name);

	test = "int func_ptr(double **foo, double **foo, double foo)";
	printf("Generating var info: %s\n", test);
	get_var_info(test, &name);

	test = "stack int foobar";
	printf("Generating var info: %s\n", test);
	get_var_info(test, &name);
	
	test = "heap int foobar";
	printf("Generating var info: %s\n", test);
	get_var_info(test, &name);

	test = "mutable stack int foobar";
	printf("Generating var info: %s\n", test);
	get_var_info(test, &name);
}



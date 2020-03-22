//
// Curly
// main.c: Runs the command line interface.
//
// jenra
// March 3 2020
//

#include <editline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/frontend/ir-codegen/gencode.h"
#include "compiler/frontend/parse/parser.h"

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		puts("Must have a string argument");
		return -1;
	}

	// Initialise the parser
	parser_t parser = create_lang_parser();

	// Parse the file
	parse_result_t res = parse_string(parser, argv[1]);
	print_parse_result(res);

	if (res.succ)
	{
		// Convert the result into ir
		compiler_t state;
		convert_tree_ir(&state, &res);

		// Print out the result
		if (state.cause == NULL)
			print_ir(&state.ir);

		// Clean up
		clean_ir(&state.ir);
	}

	// Clean up
	clean_parse_result(&res);
	clean_combinator(parser.comb);
	return 0;
}

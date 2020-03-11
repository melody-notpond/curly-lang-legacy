//
// Curly
// main.c: Runs the command line interface.
//
// jenra
// March 3 2020
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/frontend/parser.h"

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		puts("input must have at least one argument");
		return -1;
	}

	parser_t parser = create_lang_parser();

	parse_result_t res = parse_file(parser, argv[1]);
	print_parse_result(res);

	clean_parse_result(&res);
	clean_combinator(parser.comb);
	return 0;
}

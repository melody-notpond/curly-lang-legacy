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

#include "parser.h"

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		puts("input at least one argument");
		return -1;
	}

	comb_t* parser = create_lang_parser();
	parse_result_t res = parse_string(parser, argv[1]);
	print_parse_result(res);
	clean_parse_result(&res);
	clean_combinator(parser);
	return 0;
}

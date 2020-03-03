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

#include "combinator.h"

int main(int argc, char** argv)
{
	// Just to test importing the combinator library
	comb_t* comb = c_char('a');
	parse_result_t res = parse(comb, "a");
	print_parse_result(res);
	clean_parse_result(&res);
	clean_combinator(comb);
	return 0;
}

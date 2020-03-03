//
// Curly
// parser.c: Parses a given curly program.
//
// jenra
// March 3 2020
//

#include "parser.h"

// create_lang_parser(void) -> comb_t*
// Creates the parser for the language.
comb_t* create_lang_parser()
{
	comb_t* integer = c_name("int", c_regex("-?[0-9]+"));
	comb_t* floating = c_name("float", c_regex("-?[0-9]+(\\.[0-9]+([eE][+-]?[0-9]+)?|(\\.[0-9]+)?[eE][+-]?[0-9]+)"));
	comb_t* value = c_or(floating, integer, NULL);

	comb_t* parser = c_eof(value);
	parser->ignore_whitespace = true;
	return parser;
}

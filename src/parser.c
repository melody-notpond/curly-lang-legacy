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
	comb_t* expr = init_combinator();

	comb_t* integer = c_name("int", c_regex("-?[0-9]+"));
	comb_t* floating = c_name("float", c_regex("-?[0-9]+(\\.[0-9]+([eE][+-]?[0-9]+)?|(\\.[0-9]+)?[eE][+-]?[0-9]+)"));
	comb_t* character = c_name("char", c_regex("'([^'\\\\]|\\\\(x[0-9a-fA-F]{2}|[^x]))'"));
	comb_t* symbol = c_name("symbol", c_regex("[_a-zA-Z][_a-zA-Z0-9]*'*"));
	comb_t* value = c_or(
		floating, integer, character, symbol,
		c_seq(c_ignore(c_char('(')), expr, c_ignore(c_char(')')), NULL),
		NULL
	);

	comb_t* mult = c_name("infix", c_seq(
		value, c_zmore(
			c_seq(c_name("op", c_regex("[/*]")), value, NULL)
		), NULL
	));

	comb_t* add = c_name("infix", c_seq(
		mult, c_zmore(
			c_seq(c_name("op", c_regex("[+-]")), mult, NULL)
		), NULL
	));

	c_set(expr, c_name("expr", add));

	comb_t* parser = c_eof(c_name("root", c_zmore(expr)));
	parser->ignore_whitespace = true;
	return parser;
}

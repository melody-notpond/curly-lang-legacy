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

	comb_t* primatives = c_name("prim", c_regex("true|false|nil|pass|stop"));
	comb_t* integer = c_name("int", c_regex("-?[0-9]+"));
	comb_t* decimal = c_name("float", c_regex("-?[0-9]+(\\.[0-9]+([eE][+-]?[0-9]+)?|(\\.[0-9]+)?[eE][+-]?[0-9]+)"));
	comb_t* character = c_name("char", c_regex("'([^'\\\\]|\\\\(x[0-9a-fA-F]{2}|[^x]))'"));
	comb_t* symbol = c_name("symbol", c_regex("[_a-zA-Z][_a-zA-Z0-9]*'*"));
	comb_t* value = c_or(
		primatives, decimal, integer, character, symbol,
		c_seq(c_ignore(c_char('(')), expr, c_ignore(c_char(')'))),
		NULL
	);

	comb_t* prefix = c_name("prefix", c_seq(
		c_optional(c_name("op", c_char('-'))), value
	));

	comb_t* mult = c_name("infix", c_seq(
		value, c_zmore(
			c_seq(c_name("op", c_regex("[/*]")), prefix)
		)
	));

	comb_t* add = c_name("infix", c_seq(
		mult, c_zmore(
			c_seq(c_name("op", c_regex("[+-]")), mult)
		)
	));

	comb_t* assign = c_name("assign", c_seq(
		symbol, c_ignore(c_char('=')), expr
	));

	comb_t* with = c_name("with", c_seq(
		c_ignore(c_str("with")), c_omore(c_seq(assign, c_char(',')))
	));

	comb_t* if_state = c_name("if", c_seq(
		c_ignore(c_str("if")), c_name("cond", expr),
		c_ignore(c_str("then")), c_name("then", expr),
		c_optional(c_seq(
			c_ignore(c_str("else")), c_name("else", expr)
		))
	));

	comb_t* for_loop = c_name("for", c_seq(
		c_ignore(c_str("for")), c_optional(c_name("quantif", c_regex("all|some"))),
			symbol, c_ignore(c_str("in")), c_name("iter", expr),
		c_name("body", expr)
	));

	comb_t* such_that = c_name("st", c_seq(
		symbol, c_ignore(c_str("in")), c_name("iter", expr),
		c_ignore(c_regex("such[:blank:]*that")),
		c_name("cond", expr)
	));

	comb_t* comprehension;

	c_set(expr, c_or(assign, add));

	comb_t* parser = c_eof(c_name("root", c_zmore(expr)));
	parser->ignore_whitespace = true;
	return parser;
}

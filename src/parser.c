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
	comb_t* assign = init_combinator();
	comb_t* comprehension = init_combinator();

	comb_t* primatives = c_name("prim", c_regex("true|false|nil|pass|stop"));
	comb_t* integer = c_name("int", c_regex("-?[0-9]+"));
	comb_t* decimal = c_name("float", c_regex("-?[0-9]+(\\.[0-9]+([eE][+-]?[0-9]+)?|(\\.[0-9]+)?[eE][+-]?[0-9]+)"));
	comb_t* character = c_name("char", c_regex("'([^'\\\\]|\\\\(x[0-9a-fA-F]{2}|[^x]))'"));
	comb_t* symbol = c_name("symbol", c_seq(
		c_not(c_regex("(for|all|some|if|then|else|such|that|in|and|or|with)[^_a-zA-Z0-9']")),
		c_regex("[_a-zA-Z][_a-zA-Z0-9]*'*")
	));

	comb_t* range = c_name("range", c_seq(
		c_ignore(c_char('(')),
		c_optional(expr), c_char(':'), c_optional(expr),
		c_optional(c_seq(c_char(':'), expr)),
		c_ignore(c_char(')'))
	));

	comb_t* if_state = c_name("if", c_seq(
		c_ignore(c_str("if")), expr,
		c_ignore(c_str("then")), assign,
		c_optional(c_seq(
			c_ignore(c_str("else")), assign
		))
	));

	comb_t* quantifier = c_name("quantifier", c_seq(
		c_ignore(c_str("for")), c_name("quantifier", c_regex("all|some")),
			 symbol, c_ignore(c_str("in")), expr,
		assign
	));

	comb_t* value = c_or(
		if_state, quantifier,
		primatives, decimal, integer, character, symbol,
		comprehension, range,
		c_seq(c_ignore(c_char('(')), expr, c_ignore(c_char(')')))
	);

	comb_t* prefix = c_name("prefix", c_seq(
		c_optional(c_name("op", c_char('-'))), value
	));

	comb_t* mult = c_name("infix", c_seq(
		value, c_zmore(
			c_seq(c_name("op", c_regex("[/*%]")), prefix)
		)
	));

	comb_t* add = c_name("infix", c_seq(
		mult, c_zmore(
			c_seq(c_name("op", c_regex("[+-]")), mult)
		)
	));

	comb_t* compare = c_name("compare", c_seq(
		add, c_zmore(
			c_seq(c_name("op", c_regex("[><]=?|[=!]=")), add)
		)
	));

	comb_t* and = c_name("and", c_seq(
		compare, c_zmore(
			c_seq(c_ignore(c_str("and")), compare)
		)
	));

	comb_t* or = c_name("or", c_seq(
		and, c_zmore(
			c_seq(c_ignore(c_str("or")), and)
		)
	));

	comb_t* application = c_name("apply", c_seq(
		or, c_zmore(c_seq(
			c_or(c_seq(c_ignore(c_char('\\')), c_newline()), c_not(c_newline())), or
		))
	));

	comb_t* with_vars = c_seq(
		c_ignore(c_str("with")), c_omore(c_seq(assign, c_ignore(c_char(','))))
	);

	comb_t* for_loop = c_name("for", c_seq(
		c_ignore(c_str("for")), symbol, c_ignore(c_str("in")), expr,
		assign
	));

	comb_t* such_that = c_name("such that", c_seq(
		symbol, c_ignore(c_str("in")), expr,
		c_ignore(c_regex("such[[:space:]]+that")),
		expr
	));

	comb_t* comp_body = c_or(
		c_name("with",
			c_seq(c_optional(with_vars), for_loop)
		), such_that
	);

	c_set(comprehension, c_name("comprehension", c_or(
		c_seq(c_char('['), comp_body, c_char(']')),
		c_seq(c_char('('), comp_body, c_char(')')),
		c_seq(c_char('{'), comp_body, c_char('}'))
	)));

	c_set(assign, c_name("assign", c_seq(
		c_optional(c_seq(
			symbol, c_ignore(c_char('='))
		)), c_name("with", c_seq(
			c_optional(with_vars),
			expr
		))
	)));

	c_set(expr, application);

	comb_t* parser = c_eof(c_name("root", c_omore(
		c_seq(assign, c_newline())
	)));
	parser->ignore_whitespace = true;
	return parser;
}

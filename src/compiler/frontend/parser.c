//
// Curly
// parser.c: Parses a given curly program.
//
// jenra
// March 3 2020
//

#include "parser.h"
#include "lex.h"

// create_lang_parser(void) -> parser_t
// Creates the parser for the language.
parser_t create_lang_parser()
{
	comb_t* expr = init_combinator();
	comb_t* assign = init_combinator();
	comb_t* comprehension = init_combinator();

	comb_t* primatives = c_name("prim", c_type(LEX_TYPE_PRIMATIVE));
	comb_t* integer = c_name("int", c_type(LEX_TYPE_INT));
	comb_t* decimal = c_name("float", c_type(LEX_TYPE_DECIMAL));
	comb_t* symbol = c_name("symbol", c_type(LEX_TYPE_SYMBOL));

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

	comb_t* simple_value = c_or(
		primatives, decimal, integer, symbol,
		c_seq(c_ignore(c_char('(')), expr, c_ignore(c_char(')')))
	);

	comb_t* affix = c_name("affix", c_seq(
		c_optional(c_name("op", c_char('-'))), simple_value,
		c_optional(c_name("op", c_type(LEX_TYPE_POSTFIX)))
	));

	comb_t* range = c_name("range", c_seq(
		c_optional(c_seq(affix, c_not(c_newline(true)))), c_str(".."),
		c_optional(c_seq(c_not(c_newline(true)), affix)),
		c_optional(c_seq(c_not(c_newline(true)), c_char(':'), affix))
	));

	comb_t* value = c_or(
		if_state, quantifier,
		comprehension, range,
		affix
	);

	comb_t* mult = c_name("infix", c_seq(
		value, c_zmore(
			c_seq(c_name("op", c_type(LEX_TYPE_INFIX_LEVEL_MUL)), value)
		)
	));

	comb_t* add = c_name("infix", c_seq(
		mult, c_zmore(
			c_seq(c_name("op", c_type(LEX_TYPE_INFIX_LEVEL_ADD)), mult)
		)
	));

	comb_t* compare = c_name("compare", c_seq(
		add, c_zmore(
			c_seq(c_name("op", c_type(LEX_TYPE_INFIX_LEVEL_COMPARE)), add)
		)
	));

	comb_t* in = c_name("in", c_seq(
		compare, c_optional(
			c_seq(c_ignore(c_str("in")), compare)
		)
	));

	comb_t* and = c_name("and", c_seq(
		in, c_zmore(
			c_seq(c_ignore(c_str("and")), in)
		)
	));

	comb_t* or = c_name("or", c_seq(
		and, c_zmore(
			c_seq(c_ignore(c_str("or")), and)
		)
	));

	comb_t* application = c_name("apply", c_seq(
		or, c_name("args", c_zmore(c_seq(
			c_or(
				c_seq(c_ignore(c_char('\\')), c_newline(false)),
				c_not(c_newline(true))
			), or
		)))
	));

	comb_t* with_vars = c_seq(
		c_ignore(c_str("with")), c_omore(c_seq(assign, c_ignore(c_char(','))))
	);

	comb_t* for_loop = c_name("for", c_seq(
		c_ignore(c_str("for")), symbol, c_ignore(c_str("in")), value,
		assign
	));

	comb_t* such_that = c_name("such that", c_seq(
		symbol, c_ignore(c_str("in")), expr,
		c_ignore(c_seq(c_str("such"), c_str("that"))),
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
			symbol, c_name("args", c_zmore(symbol)),
			c_ignore(c_char('='))
		)), c_name("with", c_seq(
			c_optional(with_vars),
			expr
		))
	)));

	c_set(expr, application);

	comb_t* root = c_eof(c_name("root", c_omore(
		c_seq(assign, c_newline(true))
	)));

	parser_t parser = init_parser(root, false, curly_lexer_func);
	return parser;
}

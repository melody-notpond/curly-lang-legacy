//
// Curly parser combinator
// test.c: Tests various parsers.
// 
// jenra
// February 25 2020
//

#include <assert.h>
#include <stdio.h>

#include "parse/combinator.h"

void test_parsing(comb_t* comb, char* string, bool succ)
{
	printf("Parsing \"%s\":\n", string);
	parse_result_t res = parse(comb, string);
	assert((res.succ ^ !succ));
	print_parse_result(res);
	clean_parse_result(&res);
	puts("");
}

void test_c_char()
{
	// match the character a
	comb_t* comb = c_char('a');

	puts("Testing c_char(char)");
	test_parsing(comb, "a", true);
	test_parsing(comb, "b", false);
	puts("Testing complete!\n");

	clean_combinator(comb);
}

void test_c_str()
{
	// match all strings that start with owo
	comb_t* comb = c_str("owo");

	puts("Testing c_str(char*)");
	test_parsing(comb, "owouwu", true);
	test_parsing(comb, "uwuowo", false);
	test_parsing(comb, "no", false);
	test_parsing(comb, "owo", true);
	puts("Testing complete!\n");

	clean_combinator(comb);
}

void test_c_regex()
{
	// match the regex for numbers
	comb_t* comb = c_regex("-?[0-9]+(.[0-9]+)?(e[-+]?[0-9]+)?");

	puts("Testing c_regex(char*)");
	test_parsing(comb, "9843", true);
	test_parsing(comb, "-6.02", true);
	test_parsing(comb, "9.5e+4c", true);
	test_parsing(comb, "a8", false);
	test_parsing(comb, "a\n8", false);
	puts("Testing complete!\n");

	clean_combinator(comb);
}

void test_c_or()
{
	// match all strings that start with either owo, uwu, or uwo
	comb_t* comb = c_or(c_str("owo"), c_str("uwu"), c_str("uwo"), NULL);

	puts("Testing c_or(comb_t*, comb_t*, ...)");
	test_parsing(comb, "owo", true);
	test_parsing(comb, "uwu", true);
	test_parsing(comb, "uwo", true);
	test_parsing(comb, "owu", false);
	puts("Testing complete!\n");

	clean_combinator(comb);
}

void test_c_seq()
{
	// match the regex [ou]w[ou]
	comb_t* eye = c_or(c_char('o'), c_char('u'), NULL);
	comb_t* comb = c_seq(eye, c_char('w'), eye, NULL);

	puts("Testing c_seq(comb_t*, comb_t*, ...)");
	test_parsing(comb, "owo", true);
	test_parsing(comb, "uwu", true);
	test_parsing(comb, "uwo", true);
	test_parsing(comb, "owu", true);
	test_parsing(comb, "uw", false);
	puts("Testing complete!\n");

	clean_combinator(comb);
}

void test_c_zmore()
{
	// match the regex a*
	comb_t* comb = c_zmore(c_char('a'));

	puts("Testing c_zmore(comb_t*)");
	test_parsing(comb, "a", true);
	test_parsing(comb, "aaaaa", true);
	test_parsing(comb, "aaaaaaaaaaa", true);
	test_parsing(comb, "no", true);
	puts("Testing complete!\n");

	clean_combinator(comb);
}

void test_c_omore()
{
	// match the regex a+
	comb_t* comb = c_omore(c_char('a'));

	puts("Testing c_omore(comb_t*)");
	test_parsing(comb, "a", true);
	test_parsing(comb, "aaaaa", true);
	test_parsing(comb, "aaaaaaaaaaa", true);
	test_parsing(comb, "no", false);
	puts("Testing complete!\n");

	clean_combinator(comb);
}

void test_c_optional()
{
	// match the regex a?
	comb_t* comb = c_optional(c_char('a'));

	puts("Testing c_optional(comb_t*)");
	test_parsing(comb, "a", true);
	test_parsing(comb, "no", true);
	puts("Testing complete!\n");

	clean_combinator(comb);
}

void test_c_not()
{
	// match the regex [^a]
	comb_t* comb = c_not(c_char('a'));

	puts("Testing c_not(comb_t*)");
	test_parsing(comb, "a", false);
	test_parsing(comb, "b", true);
	puts("Testing complete!\n");

	clean_combinator(comb);
}

void test_c_name()
{
	// match the regex a*(b*).
	comb_t* comb = c_seq(c_zmore(c_char('a')), c_name("the bees", c_zmore(c_char('b'))), c_next(), NULL);

	puts("Testing c_name(char*, comb_t*)");
	test_parsing(comb, "aaaabbbx", true);
	puts("Testing complete!\n");

	clean_combinator(comb);
}

void test_parens()
{
	// match proper parentheses, ignoring whitespace
	comb_t* parens = init_combinator();
	c_set(parens, c_name("parens", c_seq(
		c_char('('),
			c_zmore(parens),
		c_char(')'),
		NULL
	)));
	parens = c_name("expr", c_eof(c_omore(parens)));
	parens->ignore_whitespace = true;

	puts("Testing parentheses parser");
	test_parsing(parens, "   ( )   ", true);
	test_parsing(parens, "\n\r(  ( )\t)     ( )  (  (  )\n)   ", true);
	test_parsing(parens, "(()) )", false);
	test_parsing(parens, "( (())", false);
	puts("Testing complete!\n");

	clean_combinator(parens);
}

int main(int argc, char** argv)
{
	test_c_char();
	test_c_str();
	test_c_regex();
	test_c_or();
	test_c_seq();
	test_c_zmore();
	test_c_omore();
	test_c_optional();
	test_c_not();
	test_c_name();
	test_parens();
	return 0;
}

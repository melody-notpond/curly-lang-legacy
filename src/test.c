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

void test_c_str()
{
	// match all strings that start with owo
	comb_t* comb = c_str("owo");
	parse_result_t res;

	puts("Testing c_str(char*)");
	res = parse(comb, "owouwu"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "uwuowo"); // error
	assert(!res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "owo"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	clean_combinator(comb);
	puts("");
}

void test_c_or()
{
	// match all strings that start with either owo, uwu, or uwo
	comb_t* comb = c_or(c_str("owo"), c_str("uwu"), c_str("uwo"), NULL);
	parse_result_t res;

	puts("Testing c_or(comb_t*, comb_t*, ...)");
	res = parse(comb, "owo"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "uwu"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "uwo"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "owu"); // error
	assert(!res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	clean_combinator(comb);
	puts("");
}

void test_c_seq()
{
	// match the regex [ou]w[ou]
	comb_t* eye = c_or(c_str("o"), c_str("u"), NULL);
	comb_t* comb = c_seq(eye, c_str("w"), eye, NULL);
	parse_result_t res;

	puts("Testing c_seq(comb_t*, comb_t*, ...)");
	res = parse(comb, "owo"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "uwu"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "uwo"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "owu"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "uw"); // error
	assert(!res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	clean_combinator(comb);
	puts("");
}

void test_c_zmore()
{
	// match the regex a*
	comb_t* comb = c_zmore(c_str("a"));
	parse_result_t res;

	puts("Testing c_zmore(comb_t*)");
	res = parse(comb, "a"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "aaaaa"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "aaaaaaaaaaa"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "no"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	clean_combinator(comb);
	puts("");
}

void test_c_omore()
{
	// match the regex a+
	comb_t* comb = c_omore(c_str("a"));
	parse_result_t res;

	puts("Testing c_omore(comb_t*)");
	res = parse(comb, "a"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "aaaaa"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "aaaaaaaaaaa"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "no"); // fail
	assert(!res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	clean_combinator(comb);
	puts("");
}

void test_c_optional()
{
	// match the regex a?
	comb_t* comb = c_optional(c_str("a"));
	parse_result_t res;

	puts("Testing c_optional(comb_t*)");
	res = parse(comb, "a"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "no"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	clean_combinator(comb);
	puts("");
}

void test_c_not()
{
	// match the regex [^a]
	comb_t* comb = c_not(c_str("a"));
	parse_result_t res;

	puts("Testing c_not(comb_t*)");
	res = parse(comb, "a"); // error
	assert(!res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	res = parse(comb, "b"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	clean_combinator(comb);
	puts("");
}

void test_c_name()
{
	// match the regex a*(b*).
	comb_t* comb = c_seq(c_zmore(c_str("a")), c_name("the bees", c_zmore(c_str("b"))), c_next(), NULL);
	parse_result_t res;

	puts("Testing c_name(char*, comb_t*)");
	res = parse(comb, "aaaabbbx"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	clean_combinator(comb);
	puts("");
}

void test_parens()
{
	comb_t* parens = init_combinator();
	c_set(parens, c_name("parens", c_seq(
		c_str("("), c_zmore(parens), c_str(")"), NULL
	)));
	parens = c_name("expr", c_seq(c_zmore(parens), c_eof(), NULL));
	parse_result_t res;

	puts("Testing parentheses parser");
	res = parse(parens, "()"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);
	
	res = parse(parens, "(())()(())"); // success
	assert(res.succ);
	print_parse_result(res);
	clean_parse_result(&res);
	
	res = parse(parens, "(()))"); // error
	assert(!res.succ);
	print_parse_result(res);
	clean_parse_result(&res);
	
	res = parse(parens, "((())"); // error
	assert(!res.succ);
	print_parse_result(res);
	clean_parse_result(&res);

	clean_combinator(parens);
	puts("");
}

int main(int argc, char** argv)
{
	test_c_str();
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

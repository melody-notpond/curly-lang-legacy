// 
// parse
// parser.c: Implements the parser for curly.
// 
// Created by jenra.
// Created on August 28 2020.
// 

#include <stdio.h>
#include <string.h>

#include "../../../utils/list.h"
#include "parser.h"

// succ_result(ast_t*) -> parse_result_t
// Returns a parse result containing the ast node.
parse_result_t succ_result(ast_t* ast)
{
	parse_result_t res;
	res.succ = true;
	res.ast = ast;
	return res;
}

// err_result(bool, token_t, char*) -> parse_result_t
// Returns a parse result containing an error.
parse_result_t err_result(bool fatal, token_t token, char* msg)
{
	error_t* err = malloc(sizeof(error_t));
	err->fatal = fatal;
	err->value = token;
	err->value.value = strdup(token.value);
	err->message = strdup(msg);

	parse_result_t res;
	res.succ = false;
	res.error = err;
	return res;
}

// consume_string(lexer_t*, char*, bool) -> parse_result_t
// Consumes a string from the lexer.
parse_result_t consume_string(lexer_t* lex, char* string, bool fatal)
{
	// Next token
	token_t* token = lex_next(lex);

	// Check if the token matches the string
	if (!strcmp(token->value, string))
		return succ_result(init_ast(*token));

	// Check for a lexer error
	else if (token->type == LEX_TYPE_NONE)
		return err_result(true, *token, strdup("Invalid token encountered"));

	// Return an error
	else
	{
		char* msg = "Expected '' , got '%s'";
		char* buffer = malloc(strlen(string) + strlen(msg) + 1);
		strncpy(buffer, msg, 10);
		strcpy(buffer + 10, string);
		strcpy(buffer + 10 + strlen(string), msg + 10);
		return err_result(fatal, *token, buffer);
	}
}

// consume_type(lexer_t*, lex_type_t, bool) -> parse_result_t
// Consumes a type from the lexer.
parse_result_t consume_type(lexer_t* lex, lex_type_t type, bool fatal)
{
	// Next token
	token_t* token = lex_next(lex);

	// Check if the token matches the type
	if (token->type == type)
		return succ_result(init_ast(*token));

	// Check for a lexer error
	else if (token->type == LEX_TYPE_NONE)
		return err_result(true, *token, strdup("Invalid token encountered"));

	// Return an error
	else
	{
		return err_result(fatal, *token, strdup("Expected type, got '%s'"));
	}
}

// consume_tag(lexer_t*, lex_tag_t, bool) -> parse_result_t
// Consumes a tag from the lexer.
parse_result_t consume_tag(lexer_t* lex, lex_tag_t tag, bool fatal)
{
	// Next token
	token_t* token = lex_next(lex);

	// Check if the token matches the tag
	if (token->tag == tag)
		return succ_result(init_ast(*token));

	// Check for a lexer error
	else if (token->type == LEX_TYPE_NONE)
		return err_result(true, *token, strdup("Invalid token encountered"));

	// Return an error
	else
	{
		return err_result(fatal, *token, strdup("Expected tag, got '%s'"));
	}
}

#define push_lexer(lex) size_t prev_token_pos = lex->token_pos
#define repush_lexer(lex) prev_token_pos = lex->token_pos

// consume(parse_result_t, bool, string|type|tag, lexer_t*, ?, parse_result_t, bool) -> void
// Consumes a token from the lexer.
#define consume(res, required, type, lex, arg, built, fatal_)	\
	parse_result_t res = consume_##type(lex, arg, fatal_);		\
	if (!res.succ)												\
	{															\
		(lex)->token_pos = prev_token_pos;						\
		if (res.error->fatal || (required))						\
		{														\
			clean_parse_result(built);							\
			return res;											\
		}														\
	}

// call(parse_result_t, bool, func, lexer_t*, parse_result_t) -> void
// Calls a function and crashes if a fatal error occurs.
#define call(res, required, func, lex, built, fatal_)	\
	parse_result_t res = func(lex);						\
	if (!res.succ)										\
	{													\
		(lex)->token_pos = prev_token_pos;				\
		if (fatal_)										\
			res.error->fatal = fatal_;					\
		if (res.error->fatal || (required))				\
		{												\
			clean_parse_result(built);					\
			return res;									\
		}												\
	}

// expression: 'pass' | 'stop' | with_expr | if_expr | for_loop | xor
parse_result_t expression(lexer_t* lex);

// application: expression+
parse_result_t application(lexer_t* lex);

// for_loop: 'for' symbol 'in' expression application
parse_result_t for_loop(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume for
	consume(fory, true, string, lex, "for", (parse_result_t) {false}, false);

	{
		// Push the lexer
		push_lexer(lex);

		// Check for a quantifier
		consume(all, false, string, lex, "all", fory, false);
		if (all.succ)
			list_append_element(fory.ast->children, fory.ast->children_size, fory.ast->children_count, ast_t*, all.ast);
		else
		{
			clean_parse_result(all);
			consume(some, false, string, lex, "some", fory, false);
			if (some.succ)
				list_append_element(fory.ast->children, fory.ast->children_size, fory.ast->children_count, ast_t*, some.ast);
		}
	}

	// Consume the variable name
	consume(symbol, true, type, lex, LEX_TYPE_SYMBOL, fory, true);
	list_append_element(fory.ast->children, fory.ast->children_size, fory.ast->children_count, ast_t*, symbol.ast);

	// Consume the iterator
	consume(in, true, string, lex, "in", fory, true);
	clean_parse_result(in);
	call(iter, true, expression, lex, fory, true);
	list_append_element(fory.ast->children, fory.ast->children_size, fory.ast->children_count, ast_t*, iter.ast);

	// Consume the body
	call(body, true, application, lex, fory, true);
	list_append_element(fory.ast->children, fory.ast->children_size, fory.ast->children_count, ast_t*, body.ast);
	return fory;
}

// where_expr: symbol 'in' expression 'where' application
parse_result_t where_expr(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume a symbol
	consume(symbol, true, type, lex, LEX_TYPE_SYMBOL, (parse_result_t) {false}, false);

	// Consume the in operator and form the tree
	consume(in, true, string, lex, "in", symbol, false);
	in.ast->children_size = 2;
	in.ast->children = calloc(2, sizeof(ast_t*));
	list_append_element(in.ast->children, in.ast->children_size, in.ast->children_count, ast_t*, symbol.ast);

	// Consume the iterator
	call(iter, true, expression, lex, in, true);
	list_append_element(in.ast->children, in.ast->children_size, in.ast->children_count, ast_t*, iter.ast);

	// Consume the where operator and form the tree
	consume(where, true, string, lex, "where", in, true);
	where.ast->children_size = 2;
	where.ast->children = calloc(2, sizeof(ast_t*));
	list_append_element(where.ast->children, where.ast->children_size, where.ast->children_count, ast_t*, in.ast);

	// Consume the predicate
	call(pred, true, application, lex, in, true);
	list_append_element(where.ast->children, where.ast->children_size, where.ast->children_count, ast_t*, pred.ast);
	return where;
}

// list_expr: '[' (for_loop | where_expr | (application (',' application?)*)?) ']'
parse_result_t list_expr(lexer_t* lex)
{
	// Push lexer
	push_lexer(lex);

	// Consume left bracket
	consume(lbrack, true, string, lex, "[", (parse_result_t) {false}, false);
	lbrack.ast->children_size = 1;
	lbrack.ast->children = calloc(1, sizeof(ast_t*));
	repush_lexer(lex);

	// Try to consume a list comprehension
	call(fory, false, for_loop, lex, lbrack, false);
	if (fory.succ)
	{
		// Add for loop to the tree and get right bracket
		list_append_element(lbrack.ast->children, lbrack.ast->children_size, lbrack.ast->children_count, ast_t*, fory.ast);
		consume(rbrack, true, string, lex, "]", lbrack, true);
		clean_parse_result(rbrack);
		return lbrack;
	} else clean_parse_result(fory);

	// Try to consume a list filter
	call(where, false, where_expr, lex, lbrack, false);
	if (where.succ)
	{
		// Add where expression to the tree and get right bracket
		list_append_element(lbrack.ast->children, lbrack.ast->children_size, lbrack.ast->children_count, ast_t*, where.ast);
		consume(rbrack, true, string, lex, "]", lbrack, true);
		clean_parse_result(rbrack);
		return lbrack;
	} else clean_parse_result(where);

	// Try to collect items for the list
	call(app, false, application, lex, lbrack, false);
	if (app.succ)
	{
		// Collect items
		list_append_element(lbrack.ast->children, lbrack.ast->children_size, lbrack.ast->children_count, ast_t*, app.ast);
		while (true)
		{
			// Push lexer
			push_lexer(lex);

			// Consume comma
			consume(comma, false, type, lex, LEX_TYPE_COMMA, lbrack, false);
			clean_parse_result(comma);
			if (!comma.succ) break;
			repush_lexer(lex);

			// Consume item
			call(app, false, application, lex, lbrack, false);
			if (app.succ)
				list_append_element(lbrack.ast->children, lbrack.ast->children_size, lbrack.ast->children_count, ast_t*, app.ast);
			else clean_parse_result(app);
		}
	}

	// Consume right bracket
	consume(rbrack, true, string, lex, "]", lbrack, true);
	clean_parse_result(rbrack);
	return lbrack;
}

// dict_item: symbol '=' application
parse_result_t dict_item(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume a symbol
	consume(symbol, true, type, lex, LEX_TYPE_SYMBOL, (parse_result_t) {false}, false);

	// Consume an equal sign and create the tree
	consume(assign, true, type, lex, LEX_TYPE_ASSIGN, symbol, true);
	assign.ast->children_size = 2;
	assign.ast->children = calloc(2, sizeof(ast_t*));
	list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, symbol.ast);

	// Consume an application
	call(app, true, application, lex, assign, true);
	list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, app.ast);
	return assign;
}

// dict_expr: '{' (dict_item (',' dict_item?)*)? '}'
parse_result_t dict_expr(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume curly brace
	consume(lcurly, true, string, lex, "{", (parse_result_t) {false}, false);
	repush_lexer(lex);

	// Try to collect items for the dictionary
	call(item, false, dict_item, lex, lcurly, false);
	if (item.succ)
	{
		// Collect items
		list_append_element(lcurly.ast->children, lcurly.ast->children_size, lcurly.ast->children_count, ast_t*, item.ast);
		while (true)
		{
			// Push lexer
			push_lexer(lex);

			// Consume comma
			consume(comma, false, type, lex, LEX_TYPE_COMMA, lcurly, false);
			clean_parse_result(comma);
			if (!comma.succ) break;
			repush_lexer(lex);

			// Consume item
			call(item, false, dict_item, lex, lcurly, false);
			if (item.succ)
				list_append_element(lcurly.ast->children, lcurly.ast->children_size, lcurly.ast->children_count, ast_t*, item.ast);
			else clean_parse_result(item);
		}
	}

	// Consume right curly brace
	consume(rcurly, true, string, lex, "}", lcurly, true);
	clean_parse_result(rcurly);
	return lcurly;
}

// value: int | float | symbol | string | '(' application ')' | list_expr
parse_result_t value(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume an operand
	consume(res, false, tag, lex, LEX_TAG_OPERAND, (parse_result_t) {false}, false);

	// Return the operand if successful
	if (res.succ)
		return res;
	else clean_parse_result(res);

	// Try to consume a list
	call(list, false, list_expr, lex, (parse_result_t) {false}, false);
	if (list.succ)
		return list;
	else clean_parse_result(list);

	// Try to consume a dictionary
	call(dict, false, dict_expr, lex, (parse_result_t) {false}, false);
	if (dict.succ)
		return dict;
	else clean_parse_result(dict);

	// Consume a parenthesised application
	consume(lparen, true, string, lex, "(", (parse_result_t) {false}, false);
	clean_parse_result(lparen);
	call(app, true, application, lex, (parse_result_t) {false}, true);
	consume(rparen, true, string, lex, ")", app, true);
	clean_parse_result(rparen);
	return app;
}

#define infix_parser(name, subparser, operator)																						\
parse_result_t name(lexer_t* lex)																				\
{																												\
	/* Push the lexer */																						\
	push_lexer(lex);																							\
																												\
	/* Get left operand */																						\
	call(left, true, subparser, lex, (parse_result_t) {false}, false);											\
																												\
	while (true)																								\
	{																											\
		/* Push the lexer */																					\
		push_lexer(lex);																						\
																												\
		/* Get operator */																						\
		consume(op, false, type, lex, operator, left, false);													\
		if (!op.succ) break;																					\
																												\
		/* Add left operand to the operator ast node */															\
		op.ast->children_size = 2;																				\
		op.ast->children = calloc(2, sizeof(ast_t*));															\
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t, left.ast);	\
																												\
		/* Get right operand */																					\
		call(right, true, subparser, lex, op, true);															\
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t, right.ast);	\
																												\
		/* Set left to the current operator */																	\
		left = op;																								\
	}																											\
																												\
	/* Return the parsed expression */																			\
	return left;																								\
}

// attribute: value ('.' value)*
infix_parser(attribute, value, LEX_TYPE_DOT)

// typing: attribute (':' attribute)*
// Note that the code correctness checker will assert only one colon pair is present per series
infix_parser(typing, attribute, LEX_TYPE_COLON)

// muldiv: typing (('*'|'/') typing)*
infix_parser(muldiv, typing, LEX_TYPE_MULDIV)

// addsub: muldiv (('+'|'-') muldiv)*
infix_parser(addsub, muldiv, LEX_TYPE_ADDSUB)

// bitshift: addsub (('<<'|'>>') addsub)*
infix_parser(bitshift, addsub, LEX_TYPE_BITSHIFT)

// bitand: bitshift (('&') bitshift)*
infix_parser(bitand, bitshift, LEX_TYPE_AMP)

// bitor: bitand (('|') bitand)*
infix_parser(bitor, bitand, LEX_TYPE_BAR)

// bitxor: bitor (('^') bitor)*
infix_parser(bitxor, bitor, LEX_TYPE_CARET)

// compare: bitxor (/==|[><]=?|in/ bitxor)*
infix_parser(compare, bitxor, LEX_TYPE_COMPARE)

// and: compare (('and') compare)*
infix_parser(and, compare, LEX_TYPE_AND)

// or: and (('or') and)*
infix_parser(or, and, LEX_TYPE_OR)

// xor: or (('xor') or)*
infix_parser(xor, or, LEX_TYPE_XOR)

#undef infix_parser

// assignment: symbol '..' symbol '=' application
//           | symbol ':' symbol = application
//           | symbol (symbol ':' symbol)* '=' application
parse_result_t assignment(lexer_t* lex);

// with_expr: 'with' (assignment ',')+ application
parse_result_t with_expr(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume with keyword
	consume(with, true, string, lex, "with", (parse_result_t) {false}, false);

	// Consume one assignment and add it to the with keyword ast node
	call(assign, true, assignment, lex, with, true);
	list_append_element(with.ast->children, with.ast->children_size, with.ast->children_count, ast_t*, assign.ast);

	// Consume a comma
	consume(comma, true, type, lex, LEX_TYPE_COMMA, with, true);
	clean_parse_result(comma);

	while (true)
	{
		push_lexer(lex);

		// Consume an assignment and add it to the with keyword ast node if found
		call(assign, false, assignment, lex, with, false);
		if (!assign.succ) break;
		list_append_element(with.ast->children, with.ast->children_size, with.ast->children_count, ast_t*, assign.ast);

		// Consume a comma
		consume(comma, true, type, lex, LEX_TYPE_COMMA, with, true);
		clean_parse_result(comma);
	}

	// Get an application
	call(app, true, application, lex, with, true);
	list_append_element(with.ast->children, with.ast->children_size, with.ast->children_count, ast_t*, app.ast);
	return with;
}

// if_expr: 'if' application 'then' application ('else' application)?
parse_result_t if_expr(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume an if condition and form the tree
	consume(iffy, true, string, lex, "if", (parse_result_t) {false}, false);
	call(cond, true, application, lex, iffy, true);
	iffy.ast->children_size = 3;
	iffy.ast->children = calloc(3, sizeof(ast_t*));
	list_append_element(iffy.ast->children, iffy.ast->children_size, iffy.ast->children_count, ast_t*, cond.ast);

	// Consume the body
	consume(then, true, string, lex, "then", iffy, true);
	clean_parse_result(then);
	call(body, true, application, lex, iffy, true);
	list_append_element(iffy.ast->children, iffy.ast->children_size, iffy.ast->children_count, ast_t*, body.ast);

	// Consume the else statement if applicable
	consume(elsy, false, string, lex, "else", iffy, false);
	if (elsy.succ)
	{
		call(else_body, true, application, lex, iffy, true);
		list_append_element(iffy.ast->children, iffy.ast->children_size, iffy.ast->children_count, ast_t*, else_body.ast);
	}

	return iffy;
}

// assignment: symbol '..' symbol '=' application
//           | symbol ':' symbol = application
//           | symbol (symbol ':' symbol)* '=' application
parse_result_t assignment(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume a symbol (there must be at least one for an assignment)
	consume(symbol, true, type, lex, LEX_TYPE_SYMBOL, (parse_result_t) {false}, false);
	repush_lexer(lex);

	// Try to consume a range operator
	consume(range, false, type, lex, LEX_TYPE_RANGE, symbol, false);
	if (range.succ)
	{
		// Add the head to the range operator ast node
		range.ast->children_size = 2;
		range.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(range.ast->children, range.ast->children_size, range.ast->children_count, ast_t*, symbol.ast);

		// Consume another symbol and add it as the tail to the range operator ast node
		consume(tail, true, type, lex, LEX_TYPE_SYMBOL, range, true);
		list_append_element(range.ast->children, range.ast->children_size, range.ast->children_count, ast_t*, tail.ast);

		// Consume equal sign
		consume(assign, true, type, lex, LEX_TYPE_ASSIGN, range, true);
		assign.ast->children_size = 2;
		assign.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, range.ast);

		// Get application
		call(app, true, application, lex, assign, true);

		// Add the expression to the assignment operator ast node
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, app.ast);
		return assign;
	}

	// Try to consume a colon
	clean_parse_result(range);
	consume(colon, false, type, lex, LEX_TYPE_COLON, symbol, false);
	if (colon.succ)
	{
		// Add the variable name to the type operator ast node
		colon.ast->children_size = 2;
		colon.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, symbol.ast);

		// Consume another symbol and add it as the tail to the range operator ast node
		consume(type, true, type, lex, LEX_TYPE_SYMBOL, colon, true);
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, type.ast);

		// Consume equal sign
		consume(assign, true, type, lex, LEX_TYPE_ASSIGN, colon, true);
		assign.ast->children_size = 2;
		assign.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, colon.ast);

		// Get application
		call(app, true, application, lex, assign, true);

		// Add the application to the assignment operator ast node
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, app.ast);
		return assign;
	}

	// Try to consume a dot
	clean_parse_result(colon);
	consume(dot, false, type, lex, LEX_TYPE_DOT, symbol, false);
	if (dot.succ)
	{
		// Add the parent name to the type operator ast node
		list_append_element(dot.ast->children, dot.ast->children_size, dot.ast->children_count, ast_t*, symbol.ast);

		// Consume a value
		call(val, true, value, lex, dot, true);
		list_append_element(dot.ast->children, dot.ast->children_size, dot.ast->children_count, ast_t*, val.ast);

		while (true)
		{
			// Push the lexer
			push_lexer(lex);

			// Consume a dot
			consume(dot2, false, type, lex, LEX_TYPE_DOT, dot, false);
			if (!dot2.succ) break;

			// Consume a value
			call(val, true, value, lex, dot, true);
			list_append_element(dot.ast->children, dot.ast->children_size, dot.ast->children_count, ast_t*, val.ast);
		}

		// Consume equal sign
		consume(assign, true, type, lex, LEX_TYPE_ASSIGN, dot, true);
		assign.ast->children_size = 2;
		assign.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, dot.ast);

		// Get application
		call(app, true, application, lex, assign, true);

		// Add the application to the assignment operator ast node
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, app.ast);
		return assign;
	}

	// Try to consume arguments
	clean_parse_result(dot);
	while (true)
	{
		// Push the lexer
		push_lexer(lex);

		// Consume a symbol and add it to the symbol ast node
		consume(arg, false, type, lex, LEX_TYPE_SYMBOL, symbol, false);
		if (!arg.succ) break;
		list_append_element(symbol.ast->children, symbol.ast->children_size, symbol.ast->children_count, ast_t*, arg.ast);

		// Consume a colon and add it to the symbol ast node
		consume(colon, true, type, lex, LEX_TYPE_COLON, symbol, false);
		colon.ast->children_size = 2;
		colon.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, symbol.ast->children[symbol.ast->children_count - 1]);
		symbol.ast->children[symbol.ast->children_count - 1] = colon.ast;

		// Consume an expression and add it to the colon ast node
		call(type, true, expression, lex, symbol, false);
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, type.ast);
	}

	// Consume equal sign
	consume(assign, true, type, lex, LEX_TYPE_ASSIGN, symbol, false);
	assign.ast->children_size = 2;
	assign.ast->children = calloc(2, sizeof(ast_t*));
	list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, symbol.ast);

	// Get application
	call(app, true, application, lex, assign, true);

	// Add the expression to the assignment operator ast node
	list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, app.ast);
	return assign;
}

// expression: 'pass' | 'stop' | with_expr | if_expr | for_loop | xor
parse_result_t expression(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume pass
	consume(pass, false, string, lex, "pass", (parse_result_t) {false}, false);
	if (pass.succ)
		return pass;

	// Consume stop
	clean_parse_result(pass);
	consume(stop, false, string, lex, "stop", (parse_result_t) {false}, false);
	if (stop.succ)
		return stop;

	// Call with expression
	clean_parse_result(stop);
	call(with, false, with_expr, lex, (parse_result_t) {false}, false);
	if (with.succ)
		return with;

	// Call if expression if with expression is not applicable
	clean_parse_result(with);
	call(iffy, false, if_expr, lex, (parse_result_t) {false}, false);
	if (iffy.succ)
		return iffy;

	// Call for loop if if expression is not applicable
	clean_parse_result(iffy);
	call(fory, false, for_loop, lex, (parse_result_t) {false}, false);
	if (fory.succ)
		return fory;

	// Call xor if for loop is not applicable
	clean_parse_result(fory);
	return xor(lex);
}

// application: expression+
parse_result_t application(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume the function
	call(func, true, expression, lex, (parse_result_t) {false}, false);

	// Consume any arguments if necessary
	while (true)
	{
		// Push lexer
		push_lexer(lex);

		// Add arguments to the tree
		call(arg, false, expression, lex, func, false);
		if (!arg.succ) break;
		list_append_element(func.ast->children, func.ast->children_size, func.ast->children_count, ast_t*, arg.ast);
	}

	return func;
}

// statement: assignment | expression
parse_result_t statement(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Call assignment
	call(assign, false, assignment, lex, (parse_result_t) {false}, false);
	if (assign.succ)
		return assign;

	// Call application if assignment is not applicable
	clean_parse_result(assign);
	call(app, false, application, lex, (parse_result_t) {false}, false);
	return app;
}

#undef repush_lexer
#undef push_lexer
#undef call
#undef consume

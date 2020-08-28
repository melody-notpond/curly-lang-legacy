// 
// parse
// parser.c: Implements the parser for curly.
// 
// Created by jenra.
// Created on August 28 2020.
// 

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

// consume_string(lexer_t*, char*) -> parse_result_t
// Consumes a string from the lexer.
parse_result_t consume_string(lexer_t* lex, char* string)
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
		return err_result(false, *token, buffer);
	}
}

// consume_type(lexer_t*, lex_type_t) -> parse_result_t
// Consumes a type from the lexer.
parse_result_t consume_type(lexer_t* lex, lex_type_t type)
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
		return err_result(false, *token, strdup("Expected type, got '%s'"));
	}
}

// consume_tag(lexer_t*, lex_tag_t) -> parse_result_t
// Consumes a tag from the lexer.
parse_result_t consume_tag(lexer_t* lex, lex_tag_t tag)
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
		return err_result(false, *token, strdup("Expected tag, got '%s'"));
	}
}

#define push_lexer(lex) size_t prev_token_pos = lex->token_pos
#define repush_lexer(lex) prev_token_pos = lex->token_pos

// consume(parse_result_t, bool, string|type|tag, lexer_t*, ?, parse_result_t) -> void
// Consumes a token from the lexer.
#define consume(res, crash, type, lex, arg, built)	\
	parse_result_t res = consume_##type(lex, arg);	\
	if (!res.succ)									\
	{												\
		lex->token_pos = prev_token_pos;			\
		if (res.error->fatal || crash)				\
		{											\
			clean_parse_result(built);				\
			return res;								\
		}											\
	}

// call(parse_result_t, bool, func, lexer_t*, parse_result_t) -> void
// Calls a function and crashes if a fatal error occurs.
#define call(res, crash, func, lex, built)			\
	parse_result_t res = func(lex);					\
	if (!res.succ)									\
	{												\
		lex->token_pos = prev_token_pos;			\
		if (res.error->fatal || crash)				\
		{											\
			clean_parse_result(built);				\
			return res;								\
		}											\
	}

// expression: bitshift
parse_result_t expression(lexer_t* lex);

// value: int | float | symbol | string | '(' expression ')'
parse_result_t value(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume an operand
	consume(res, false, tag, lex, LEX_TAG_OPERAND, (parse_result_t) {false});

	// Return the operand if successful
	if (res.succ)
		return res;
	else clean_parse_result(res);

	// Consume a parenthesised expression
	consume(lparen, true, string, lex, "(", (parse_result_t) {false});
	clean_parse_result(lparen);
	call(expr, true, expression, lex, (parse_result_t) {false});
	consume(rparen, true, string, lex, ")", expr);
	clean_parse_result(rparen);
	return expr;
}

#define infix_parser(name, subparser, operator)																						\
parse_result_t name(lexer_t* lex)																				\
{																												\
	/* Push the lexer */																						\
	push_lexer(lex);																							\
																												\
	/* Get left operand */																						\
	call(left, true, subparser, lex, (parse_result_t) {false});													\
																												\
	while (true)																								\
	{																											\
		/* Push the lexer */																					\
		push_lexer(lex);																						\
																												\
		/* Get operator */																						\
		consume(op, false, type, lex, operator, left);															\
		if (!op.succ) break;																					\
																												\
		/* Add left operand to the operator ast node */															\
		op.ast->children_size = 2;																				\
		op.ast->children = calloc(2, sizeof(ast_t*));															\
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t, left.ast);	\
																												\
		/* Get right operand */																					\
		call(right, true, subparser, lex, op);																	\
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t, right.ast);	\
																												\
		/* Set left to the current operator */																	\
		left = op;																								\
	}																											\
																												\
	/* Return the parsed expression */																			\
	return left;																								\
}

// muldiv: value (('*'|'/') value)*
infix_parser(muldiv, value, LEX_TYPE_MULDIV)

// addsub: muldiv (('+'|'-') muldiv)*
infix_parser(addsub, muldiv, LEX_TYPE_ADDSUB)

// bitshift: addsub (('<<'|'>>') addsub)*
infix_parser(bitshift, addsub, LEX_TYPE_BITSHIFT)

// bitand: bitshift (('&') addsub)*
infix_parser(bitand, bitshift, LEX_TYPE_AMP)

// bitor: bitand (('|') addsub)*
infix_parser(bitor, bitand, LEX_TYPE_BAR)

// bitxor: bitor (('^') addsub)*
infix_parser(bitxor, bitor, LEX_TYPE_CARET)

// compare: bitxor (/==|[><]=?/ bitshift)*
infix_parser(compare, bitxor, LEX_TYPE_COMPARE)

// and: compare (('and') addsub)*
infix_parser(and, compare, LEX_TYPE_AND)

// or: and (('or') addsub)*
infix_parser(or, and, LEX_TYPE_OR)

// xor: or (('xor') addsub)*
infix_parser(xor, or, LEX_TYPE_XOR)

#undef infix_parser

// assignment: symbol .. symbol = expr
parse_result_t assignment(lexer_t* lex);

// with_expr: 'with' (assignment ',')+ expression
parse_result_t with_expr(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume with keyword
	consume(with, true, string, lex, "with", (parse_result_t) {false});

	// Consume one assignment and add it to the with keyword ast node
	call(assign, true, assignment, lex, with);
	list_append_element(with.ast->children, with.ast->children_size, with.ast->children_count, ast_t*, assign.ast);
	// Consume a comma
	consume(comma, true, type, lex, LEX_TYPE_COMMA, with);
	clean_parse_result(comma);

	while (true)
	{
		push_lexer(lex);

		// Consume an assignment and add it to the with keyword ast node if found
		call(assign, false, assignment, lex, with);
		if (!assign.succ) break;
		list_append_element(with.ast->children, with.ast->children_size, with.ast->children_count, ast_t*, assign.ast);

		// Consume a comma
		consume(comma, true, type, lex, LEX_TYPE_COMMA, with);
		clean_parse_result(comma);
	}

	// Get an expression
	call(expr, true, expression, lex, with);
	list_append_element(with.ast->children, with.ast->children_size, with.ast->children_count, ast_t*, expr.ast);
	return with;
}

// assignment: symbol '..' symbol '=' expression
//           | symbol ':' symbol = expression
//           | symbol (symbol ':' symbol)* '=' expression
parse_result_t assignment(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume a symbol (there must be at least one for an assignment)
	consume(symbol, true, type, lex, LEX_TYPE_SYMBOL, (parse_result_t) {false});
	repush_lexer(lex);

	// Try to consume a range operator
	consume(range, false, type, lex, LEX_TYPE_RANGE, symbol);
	if (range.succ)
	{
		// Add the head to the range operator ast node
		range.ast->children_size = 2;
		range.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(range.ast->children, range.ast->children_size, range.ast->children_count, ast_t*, symbol.ast);

		// Consume another symbol and add it as the tail to the range operator ast node
		consume(tail, true, type, lex, LEX_TYPE_SYMBOL, range);
		list_append_element(range.ast->children, range.ast->children_size, range.ast->children_count, ast_t*, tail.ast);

		// Consume equal sign
		consume(assign, true, type, lex, LEX_TYPE_ASSIGN, range);
		assign.ast->children_size = 2;
		assign.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, range.ast);

		// Get expression
		call(expr, true, expression, lex, assign);

		// Add the expression to the assignment operator ast node
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, expr.ast);
		return assign;
	}

	// Try to consume a colon
	clean_parse_result(range);
	consume(colon, false, type, lex, LEX_TYPE_COLON, symbol);
	if (colon.succ)
	{
		// Add the variable name to the type operator ast node
		colon.ast->children_size = 2;
		colon.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, symbol.ast);

		// Consume another symbol and add it as the tail to the range operator ast node
		consume(type, true, type, lex, LEX_TYPE_SYMBOL, colon);
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, type.ast);

		// Consume equal sign
		consume(assign, true, type, lex, LEX_TYPE_ASSIGN, colon);
		assign.ast->children_size = 2;
		assign.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, colon.ast);

		// Get expression
		call(expr, true, expression, lex, assign);

		// Add the expression to the assignment operator ast node
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, expr.ast);
		return assign;
	}

	// Try to consume arguments
	clean_parse_result(colon);
	while (true)
	{
		// Push the lexer
		push_lexer(lex);

		// Consume a symbol and add it to the symbol ast node
		consume(arg, false, type, lex, LEX_TYPE_SYMBOL, symbol);
		if (!arg.succ) break;
		list_append_element(symbol.ast->children, symbol.ast->children_size, symbol.ast->children_count, ast_t*, arg.ast);

		// Consume a colon and add it to the symbol ast node
		consume(colon, true, type, lex, LEX_TYPE_COLON, symbol);
		colon.ast->children_size = 2;
		colon.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, symbol.ast->children[symbol.ast->children_count - 1]);
		symbol.ast->children[symbol.ast->children_count - 1] = colon.ast;

		// Consume a symbol and add it to the colon ast node
		consume(type, true, type, lex, LEX_TYPE_SYMBOL, symbol);
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, type.ast);
	}

	// Consume equal sign
	consume(assign, true, type, lex, LEX_TYPE_ASSIGN, symbol);
	assign.ast->children_size = 2;
	assign.ast->children = calloc(2, sizeof(ast_t*));
	list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, symbol.ast);

	// Get expression
	call(expr, true, expression, lex, assign);

	// Add the expression to the assignment operator ast node
	list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, expr.ast);
	return assign;
}

// expression: with_expr | xor
parse_result_t expression(lexer_t* lex)
{
	push_lexer(lex);

	// Call with expression
	call(with, false, with_expr, lex, (parse_result_t) {false});
	if (with.succ)
		return with;

	// Call xor if with expression is not applicable
	clean_parse_result(with);
	return xor(lex);
}

// statement: assignment | expression
parse_result_t statement(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Call assignment
	call(assign, false, assignment, lex, (parse_result_t) {false});
	if (assign.succ)
		return assign;

	// Call expression if assignment is not applicable
	clean_parse_result(assign);
	call(expr, false, expression, lex, (parse_result_t) {false});
	return expr;
}

#undef repush_lexer
#undef push_lexer
#undef call
#undef consume

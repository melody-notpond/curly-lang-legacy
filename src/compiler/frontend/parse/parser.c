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

// consume(parse_result_t&, string|type|tag, lexer_t*, *) -> void
// Consumes a token from the lexer.
#define consume(res, crash, type, lex, arg)			\
	parse_result_t res = consume_##type(lex, arg);	\
	if (!res.succ)									\
	{												\
		lex->token_pos = prev_token_pos;			\
		if (res.error->fatal || crash)				\
			return res;								\
	}

// call(parse_result_t&, func, ...) -> void
// Calls a function and crashes if a fatal error occurs.
#define call(res, crash, func, lex)					\
	parse_result_t res = func(lex);					\
	if (!res.succ)									\
	{												\
		lex->token_pos = prev_token_pos;			\
		if (res.error->fatal || crash)				\
			return res;								\
	}

// expression: bitshift
parse_result_t expression(lexer_t* lex);

// value: int | float | symbol | string | '(' expression ')'
parse_result_t value(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume an operand
	consume(res, false, tag, lex, LEX_TAG_OPERAND);

	// Return the operand if successful
	if (res.succ)
		return res;

	// Consume a parenthesised expression
	consume(lparen, true, string, lex, "(");
	clean_parse_result(lparen);
	call(expr, true, expression, lex);
	consume(rparen, true, string, lex, ")");
	clean_parse_result(rparen);
	return expr;
}

// muldiv: value (('*'|'/') value)*
parse_result_t muldiv(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Get left operand
	call(left, true, value, lex);

	while (true)
	{
		// Push the lexer
		push_lexer(lex);

		// Get operator and right operand
		consume(op, false, type, lex, LEX_TYPE_MULDIV);
		if (!op.succ) break;
		call(right, true, value, lex);

		// Add left and right operands to the operator ast node
		op.ast->children_size = 2;
		op.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t, left.ast);
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t, right.ast);

		// Set left to the current operator
		left = op;
	}

	// Return the parsed expression
	return left;
}

// addsub: muldiv (('+'|'-') muldiv)*
parse_result_t addsub(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Get left operand
	call(left, true, muldiv, lex);

	while (true)
	{
		// Push the lexer
		push_lexer(lex);

		// Get operator and right operand
		consume(op, false, type, lex, LEX_TYPE_ADDSUB);
		if (!op.succ) break;
		call(right, true, muldiv, lex);

		// Add left and right operands to the operator ast node
		op.ast->children_size = 2;
		op.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t, left.ast);
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t, right.ast);

		// Set left to the current operator
		left = op;
	}

	// Return the parsed expression
	return left;
}

// bitshift: addsub (('<<'|'>>') addsub)*
parse_result_t bitshift(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Get left operand
	call(left, true, addsub, lex);

	while (true)
	{
		// Push the lexer
		push_lexer(lex);

		// Get operator and right operand
		consume(op, false, type, lex, LEX_TYPE_BITSHIFT);
		if (!op.succ) break;
		call(right, true, addsub, lex);

		// Add left and right operands to the operator ast node
		op.ast->children_size = 2;
		op.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t, left.ast);
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t, right.ast);

		// Set left to the current operator
		left = op;
	}

	// Return the parsed expression
	return left;
}

// expression: bitshift
parse_result_t expression(lexer_t* lex) { return bitshift(lex); }

#undef call
#undef consume

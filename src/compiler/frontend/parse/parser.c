// 
// parse
// parser.c: Implements the parser for curly.
// 
// Created by jenra.
// Created on August 28 2020.
// 

#include <string.h>

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
		strcpy(buffer + 10 + strlen(string), msg + 11);
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

// consume(parse_result_t&, string|type|tag, lexer_t*, *) -> void
// Consumes a token from the lexer.
#define consume(res, type, lex, arg)				\
	parse_result_t res = consume_##type(lex, arg);	\
	if (!res.succ && res.err.fatal)					\
		return res;									\

// call(parse_result_t&, func, ...) -> void
// Calls a function and crashes if a fatal error occurs.
#define call(res, func, ...)				\
	parse_result res = func(__VA_ARGS__)	\
	if (!res.succ && res.err.fatal)			\
		return res;							\



#undef call
#undef consume

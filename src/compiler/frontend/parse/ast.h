// 
// parse
// ast.h: Header file for ast.c.
// 
// Created by jenra.
// Created on August 27 2020.
// 

#include <stdbool.h>

#include "lexer.h"

// Represents a parsing error.
typedef struct
{
	// Whether the error should force parsing to halt or not.
	bool fatal;

	// The token that caused the error.
	token_t value;

	// The error message.
	char* message;
} error_t;

// Represents an abstract syntax tree node.
typedef struct s_ast
{
	// The token the ast node represents.
	token_t value;

	// The list of children of the ast node.
	struct s_ast** children;
	size_t children_count;
	size_t children_size;
} ast_t;

// Represents the result of parsing a string.
typedef struct
{
	// Whether the parse was successful or failed.
	bool succ;

	// The value of the parsing result.
	union
	{
		ast_t* ast;
		error_t* error;
	};
} parse_result_t;

// init_ast(token_t) -> ast_t*
// Initialises an ast.
ast_t* init_ast(token_t token);

// print_ast(ast_t*) -> void
// Prints an ast.
void print_ast(ast_t* ast);

// clean_ast(ast_t*) -> void
// Deletes an ast.
void clean_ast(ast_t* ast);

// clean_parse_result(parse_result_t) -> void
// Deletes a parse result's data.
void clean_parse_result(parse_result_t result);

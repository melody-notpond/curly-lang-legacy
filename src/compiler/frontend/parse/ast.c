// 
// parse
// ast.c: Implements an abstract syntax tree.
// 
// Created by jenra.
// Created on August 27 2020.
// 

#include <stdio.h>
#include <string.h>

#include "ast.h"

// init_ast(token_t) -> ast_t*
// Initialises an ast.
ast_t* init_ast(token_t token)
{
	ast_t* ast = malloc(sizeof(ast_t));
	ast->value = token;
	ast->value.value = token.value != NULL ? strdup(token.value) : NULL;
	ast->children = NULL;
	ast->children_count = 0;
	ast->children_size = 0;
	return ast;
}

void print_ast_helper(ast_t* ast, int level)
{
	// Tab out the ast node
	for (int i = 0; i < level; i++)
	{
		putc('\t', stdout);
	}
	if (level > 0)
		printf("| ");

	// Print out the token
	printf("%s (%i:%i/%i)\n", ast->value.value, ast->value.lino, ast->value.charpos, ast->value.type);

	// Print out children if there are any
	for (size_t i = 0; i < ast->children_count; i++)
	{
		print_ast_helper(ast->children[i], level + 1);
	}
}

// print_ast(ast_t*) -> void
// Prints an ast.
void print_ast(ast_t* ast) { print_ast_helper(ast, 0); }

// clean_ast(ast_t*) -> void
// Deletes an ast.
void clean_ast(ast_t* ast)
{
	// Don't do anything if the ast node is null
	if (ast == NULL)
		return;

	// Delete the children
	for (size_t i = 0; i < ast->children_count; i++)
	{
		clean_ast(ast->children[i]);
	}

	// Delete the fields
	free(ast->value.value);
	free(ast->children);
	free(ast);
}

// clean_parse_result(parse_result_t) -> void
// Deletes a parse result's data.
void clean_parse_result(parse_result_t result)
{
	if (result.succ)
		// Delete ast node
		clean_ast(result.ast);
	else if (result.error != NULL)
	{
		// Free error fields
		free(result.error->expected);
		free(result.error->value.value);
		free(result.error);
	}
}

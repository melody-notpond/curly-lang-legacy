// 
// ir
// generate_ir.c: Converts the abstract syntax tree into an intermediate representation used by the backends.
// 
// Created by jenra.
// Created on October 12 2020.
// 

#include <string.h>

#include "generate_ir.h"

ir_expr_t* convert_ast_node(curly_ir_t* root, ast_t* ast)
{
	ir_expr_t* expr = NULL;

	// Operands
	if (ast->value.tag == LEX_TAG_OPERAND)
	{
		;

	// Infix operators
	} else if (ast->value.tag == LEX_TAG_INFIX_OPERATOR)
	{
		;

	// Prefix operators
	} else if (!strcmp(ast->value.value, "*") || !strcmp(ast->value.value, "-"))
	{
		;

	// Assignments
	} else if (ast->value.type == LEX_TYPE_ASSIGN)
	{
		;

	// Declarations
	} else if (ast->value.type == LEX_TYPE_COLON)
	{
		;

	// With expressions
	} else if (!strcmp(ast->value.value, "with"))
	{
		;

	// If expressions
	} else if (!strcmp(ast->value.value, "if"))
	{
		;

	// Unsupported syntax
	} else return NULL;

	return expr;
}

// convert_ast_to_ir(ast_t*) -> curly_ir_t
// Converts a given ast root to ir.
curly_ir_t convert_ast_to_ir(ast_t* ast)
{
	curly_ir_t ir;
	ir.expr_count = ast->children_count;
	ir.expr = calloc(ir.expr_count, sizeof(ir_expr_t*));

	for (size_t i = 0; i < ir.expr_count; i++)
	{
		ir.expr[i] = convert_ast_node(&ir, ast->children[i]);
	}

	return ir;
}

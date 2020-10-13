// 
// ir
// generate_ir.c: Converts the abstract syntax tree into an intermediate representation used by the backends.
// 
// Created by jenra.
// Created on October 12 2020.
// 

#include <string.h>
#include <stdio.h>

#include "generate_ir.h"

// convert_ast_node(curly_ir_t*, ast_t*) -> ir_sexpr_t*
// Converts an ast node into an S expression.
ir_sexpr_t* convert_ast_node(curly_ir_t* root, ast_t* ast)
{
	ir_sexpr_t* sexpr = malloc(sizeof(ir_sexpr_t));
	sexpr->type = NULL;
	sexpr->lino = ast->value.lino;
	sexpr->charpos = ast->value.charpos;
	sexpr->pos = ast->value.pos;

	// Operands
	if (ast->value.tag == LEX_TAG_OPERAND)
	{
		switch (ast->value.type)
		{
			case LEX_TYPE_INT:
				sexpr->tag = CURLY_IR_TAGS_INT;
				sexpr->i64 = atoll(ast->value.value);
				break;
			case LEX_TYPE_FLOAT:
				sexpr->tag = CURLY_IR_TAGS_DOUBLE;
				sexpr->f64 = atof(ast->value.value);
				break;
			case LEX_TYPE_BOOL:
				sexpr->tag = CURLY_IR_TAGS_BOOL;
				sexpr->i1 = !strcmp(ast->value.value, "true");
				break;
			case LEX_TYPE_SYMBOL:
				sexpr->tag = CURLY_IR_TAGS_SYMBOL;
				sexpr->symbol = strdup(ast->value.value);
				break;
			default:
				puts("Unimplemented operand!");
				break;
		}

	// Infix operators
	} else if (ast->value.tag == LEX_TAG_INFIX_OPERATOR)
	{
		sexpr->tag = CURLY_IR_TAGS_INFIX;
		sexpr->infix.left  = convert_ast_node(root, ast->children[0]);
		sexpr->infix.right = convert_ast_node(root, ast->children[1]);
		sexpr->infix.op = !strcmp(ast->value.value, "*")   ? IR_BINOPS_MUL
					   : !strcmp(ast->value.value, "/")   ? IR_BINOPS_DIV
					   : !strcmp(ast->value.value, "%")   ? IR_BINOPS_MOD
					   : !strcmp(ast->value.value, "+")   ? IR_BINOPS_ADD
					   : !strcmp(ast->value.value, "-")   ? IR_BINOPS_SUB
					   : !strcmp(ast->value.value, "<<")  ? IR_BINOPS_BSL
					   : !strcmp(ast->value.value, ">>")  ? IR_BINOPS_BSR
					   : !strcmp(ast->value.value, "&")   ? IR_BINOPS_BITAND
					   : !strcmp(ast->value.value, "|")   ? IR_BINOPS_BITOR
					   : !strcmp(ast->value.value, "^")   ? IR_BINOPS_BITXOR
					   : !strcmp(ast->value.value, "<")   ? IR_BINOPS_CMPLT
					   : !strcmp(ast->value.value, ">")   ? IR_BINOPS_CMPGT
					   : !strcmp(ast->value.value, "<=")  ? IR_BINOPS_CMPLTE
					   : !strcmp(ast->value.value, ">=")  ? IR_BINOPS_CMPGTE
					   : !strcmp(ast->value.value, "==")  ? IR_BINOPS_CMPEQ
					   : !strcmp(ast->value.value, "!=")  ? IR_BINOPS_CMPNEQ
					   : !strcmp(ast->value.value, "and") ? IR_BINOPS_BOOLAND
					   : !strcmp(ast->value.value, "or")  ? IR_BINOPS_BOOLOR
					   : !strcmp(ast->value.value, "xor") ? IR_BINOPS_BOOLXOR
					   : -1;

	// Prefix operators
	} else if (!strcmp(ast->value.value, "*") || !strcmp(ast->value.value, "-"))
	{
		sexpr->tag = CURLY_IR_TAGS_PREFIX;
		sexpr->prefix.operand = convert_ast_node(root, ast->children[0]);
		sexpr->prefix.op = !strcmp(ast->value.value, "*") ? IR_BINOPS_SPAN
						: !strcmp(ast->value.value, "-") ? IR_BINOPS_NEG
						: -1;

	// Assignments
	} else if (ast->value.type == LEX_TYPE_ASSIGN)
	{
		sexpr->tag = CURLY_IR_TAGS_ASSIGN;
		char* name = NULL;
		ast_t* head = ast->children[0];

		if (head->value.type == LEX_TYPE_SYMBOL)
			name = head->value.value;
		else if (head->value.type == LEX_TYPE_COLON)
		{
			name = head->children[0]->value.value;

			// TODO extract types

		// TODO functions, attributes, and head/tail
		} else puts("Unsupported assignment!");

		sexpr->assign.name = strdup(name);
		sexpr->assign.value = convert_ast_node(root, ast->children[1]);

	// Declarations
	} else if (ast->value.type == LEX_TYPE_COLON)
	{
		sexpr->tag = CURLY_IR_TAGS_DECLARE;
		sexpr->declare.name = strdup(ast->children[0]->value.value);

		// TODO extract types

	// With expressions
	} else if (!strcmp(ast->value.value, "with"))
	{
		sexpr->tag = CURLY_IR_TAGS_LOCAL_SCOPE;
		sexpr->local_scope.assign_count = ast->children_count - 1;
		sexpr->local_scope.assigns = calloc(sexpr->local_scope.assign_count, sizeof(ir_sexpr_t));

		for (size_t i = 0; i < sexpr->local_scope.assign_count; i++)
		{
			sexpr->local_scope.assigns = convert_ast_node(root, ast->children[i]);
		}

		sexpr->local_scope.value = convert_ast_node(root, ast->children[ast->children_count - 1]);

	// If expressions
	} else if (!strcmp(ast->value.value, "if"))
	{
		sexpr->tag = CURLY_IR_TAGS_IF;
		sexpr->if_expr.cond = convert_ast_node(root, ast->children[0]);
		sexpr->if_expr.then = convert_ast_node(root, ast->children[1]);
		sexpr->if_expr.elsy = convert_ast_node(root, ast->children[2]);

	// Unsupported syntax
	} else return NULL;

	return sexpr;
}

// convert_ast_to_ir(ast_t*) -> curly_ir_t
// Converts a given ast root to IR.
curly_ir_t convert_ast_to_ir(ast_t* ast)
{
	curly_ir_t ir;
	ir.expr_count = ast->children_count;
	ir.expr = calloc(ir.expr_count, sizeof(ir_sexpr_t*));

	for (size_t i = 0; i < ir.expr_count; i++)
	{
		ir.expr[i] = convert_ast_node(&ir, ast->children[i]);
	}

	return ir;
}



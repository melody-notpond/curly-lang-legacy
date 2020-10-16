// 
// ir
// generate_ir.c: Converts the abstract syntax tree into an intermediate representation used by the backends.
// 
// Created by jenra.
// Created on October 12 2020.
// 

#include <string.h>
#include <stdio.h>

#include "../correctness/type_generators.h"
#include "generate_ir.h"

// convert_ast_node(curly_ir_t*, ast_t*, ir_scope_t*) -> ir_sexpr_t*
// Converts an ast node into an S expression.
ir_sexpr_t* convert_ast_node(curly_ir_t* root, ast_t* ast, ir_scope_t* scope)
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
				sexpr->tag = CURLY_IR_TAGS_FLOAT;
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
		sexpr->infix.left  = convert_ast_node(root, ast->children[0], scope);
		sexpr->infix.right = convert_ast_node(root, ast->children[1], scope);
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
		sexpr->prefix.operand = convert_ast_node(root, ast->children[0], scope);
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

			sexpr->type = generate_type(head->children[1], scope, NULL, NULL);

		// TODO functions, attributes, and head/tail
		} else puts("Unsupported assignment!");

		sexpr->assign.name = strdup(name);
		sexpr->assign.value = convert_ast_node(root, ast->children[1], scope);

	// Declarations
	} else if (ast->value.type == LEX_TYPE_COLON)
	{
		sexpr->tag = CURLY_IR_TAGS_DECLARE;
		sexpr->declare.name = strdup(ast->children[0]->value.value);

		sexpr->type = generate_type(ast->children[1], scope, NULL, NULL);

	// With expressions
	} else if (!strcmp(ast->value.value, "with"))
	{
		sexpr->tag = CURLY_IR_TAGS_LOCAL_SCOPE;
		sexpr->local_scope.assign_count = ast->children_count - 1;
		sexpr->local_scope.assigns = calloc(sexpr->local_scope.assign_count, sizeof(ir_sexpr_t));

		for (size_t i = 0; i < sexpr->local_scope.assign_count; i++)
		{
			sexpr->local_scope.assigns[i] = convert_ast_node(root, ast->children[i], scope);
		}

		sexpr->local_scope.value = convert_ast_node(root, ast->children[ast->children_count - 1], scope);

	// If expressions
	} else if (!strcmp(ast->value.value, "if"))
	{
		sexpr->tag = CURLY_IR_TAGS_IF;
		sexpr->if_expr.cond = convert_ast_node(root, ast->children[0], scope);
		sexpr->if_expr.then = convert_ast_node(root, ast->children[1], scope);
		sexpr->if_expr.elsy = convert_ast_node(root, ast->children[2], scope);

	// Unsupported syntax
	} else
	{
		puts("Unsupported syntax!");
		return NULL;
	}

	return sexpr;
}

// convert_ast_to_ir(ast_t*, ir_scope_t*) -> curly_ir_t
// Converts a given ast root to IR.
curly_ir_t convert_ast_to_ir(ast_t* ast, ir_scope_t* scope)
{
	curly_ir_t ir;
	ir.expr_count = ast->children_count;
	ir.expr = calloc(ir.expr_count, sizeof(ir_sexpr_t*));

	for (size_t i = 0; i < ir.expr_count; i++)
	{
		ir.expr[i] = convert_ast_node(&ir, ast->children[i], scope);
	}

	return ir;
}

// print_ir_sexpr(ir_sexpr_t*, int, bool) -> void
// Prints out an IR S expression to stdout.
void print_ir_sexpr(ir_sexpr_t* sexpr, int indent, bool newline)
{
	// Indent and print parenthesis
	if (newline)
	{
		for (int i = 0; i < indent; i++)
			printf("  ");
		newline = false;
	}
	printf("%s: ", sexpr->type != NULL ? sexpr->type->type_name : "(null)");
	printf("(");

	// Ad hoc match expression for printing
	switch (sexpr->tag)
	{
		case CURLY_IR_TAGS_INT:
			printf("%lli: Int", sexpr->i64);
			break;
		case CURLY_IR_TAGS_FLOAT:
			printf("%.05f: Float", sexpr->f64);
			break;
		case CURLY_IR_TAGS_BOOL:
			printf("%s: Bool", sexpr->i1 ? "true" : "false");
			break;
		case CURLY_IR_TAGS_SYMBOL:
			printf("%s: %s", sexpr->symbol, sexpr->type != NULL ? sexpr->type->type_name : "(null)");
			break;
		case CURLY_IR_TAGS_INFIX:
			printf("call(2) ");
			switch (sexpr->infix.op)
			{
				case IR_BINOPS_MUL:
					printf("*");
					break;
				case IR_BINOPS_DIV:
					printf("/");
					break;
				case IR_BINOPS_MOD:
					printf("%%");
					break;
				case IR_BINOPS_ADD:
					printf("+");
					break;
				case IR_BINOPS_SUB:
					printf("-");
					break;
				case IR_BINOPS_BSL:
					printf("<<");
					break;
				case IR_BINOPS_BSR:
					printf(">>");
					break;
				case IR_BINOPS_BITAND:
					printf("&");
					break;
				case IR_BINOPS_BITOR:
					printf("|");
					break;
				case IR_BINOPS_BITXOR:
					printf("^");
					break;
				case IR_BINOPS_CMPEQ:
					printf("==");
					break;
				case IR_BINOPS_CMPNEQ:
					printf("!=");
					break;
				case IR_BINOPS_CMPLT:
					printf("<");
					break;
				case IR_BINOPS_CMPLTE:
					printf("<=");
					break;
				case IR_BINOPS_CMPGT:
					printf(">");
					break;
				case IR_BINOPS_CMPGTE:
					printf(">=");
					break;
				case IR_BINOPS_CMPIN:
					printf("in");
					break;
				case IR_BINOPS_BOOLAND:
					printf("and");
					break;
				case IR_BINOPS_BOOLOR:
					printf("or");
					break;
				case IR_BINOPS_BOOLXOR:
					printf("xor");
					break;
				default:
					printf("???");
					break;
			}
			printf(" ");
			print_ir_sexpr(sexpr->infix.left, indent, false);
			printf(" ");
			print_ir_sexpr(sexpr->infix.right, indent, false);
			break;
		case CURLY_IR_TAGS_PREFIX:
			printf("call(1) ");
			switch (sexpr->prefix.op)
			{
				case IR_BINOPS_SPAN:
					printf("*");
					break;
				case IR_BINOPS_NEG:
					printf("-");
					break;
				default:
					printf("???");
					break;
			}
			printf(" ");
			print_ir_sexpr(sexpr->prefix.operand, indent, false);
			break;
		case CURLY_IR_TAGS_ASSIGN:
			printf("set %s\n", sexpr->assign.name);
			print_ir_sexpr(sexpr->assign.value, indent + 1, true);
			puts("");
			newline = true;
			break;
		case CURLY_IR_TAGS_DECLARE:
			printf("declare %s: %s", sexpr->declare.name, sexpr->type != NULL ? sexpr->type->type_name : "(null)");
			break;
		case CURLY_IR_TAGS_LOCAL_SCOPE:
			puts("scope");
			for (size_t i = 0; i < sexpr->local_scope.assign_count; i++)
			{
				print_ir_sexpr(sexpr->local_scope.assigns[i], indent + 1, true);
				puts("");
			}
			print_ir_sexpr(sexpr->local_scope.value, indent + 1, true);
			puts("");
			newline = true;
			break;
		case CURLY_IR_TAGS_IF:
			printf("if ");
			print_ir_sexpr(sexpr->if_expr.cond, indent, false);
			puts("");
			print_ir_sexpr(sexpr->if_expr.then, indent + 1, true);
			puts("");
			print_ir_sexpr(sexpr->if_expr.elsy, indent + 1, true);
			puts("");
			newline = true;
			break;
		default:
			printf("???");
	}

	// Indent and print parenthesis
	if (newline)
		for (int i = 0; i < indent; i++)
			printf("  ");
	printf(")");
}

// print_ir(curly_ir_t) -> void
// Prints out IR to stdout.
void print_ir(curly_ir_t ir)
{
	for (size_t i = 0; i < ir.expr_count; i++)
	{
		print_ir_sexpr(ir.expr[i], 0, true);
		puts("");
	}
}

// clean_ir_sexpr(ir_sexpr_t*) -> void
// Cleans up an IR S expression.
void clean_ir_sexpr(ir_sexpr_t* sexpr)
{
	switch (sexpr->tag)
	{
		case CURLY_IR_TAGS_SYMBOL:
			free(sexpr->symbol);
			break;
		case CURLY_IR_TAGS_INFIX:
			clean_ir_sexpr(sexpr->infix.left);
			clean_ir_sexpr(sexpr->infix.right);
			break;
		case CURLY_IR_TAGS_PREFIX:
			clean_ir_sexpr(sexpr->prefix.operand);
			break;
		case CURLY_IR_TAGS_ASSIGN:
			free(sexpr->assign.name);
			clean_ir_sexpr(sexpr->assign.value);
			break;
		case CURLY_IR_TAGS_DECLARE:
			free(sexpr->declare.name);
			break;
		case CURLY_IR_TAGS_LOCAL_SCOPE:
			for (size_t i = 0; i < sexpr->local_scope.assign_count; i++)
			{
				clean_ir_sexpr(sexpr->local_scope.assigns[i]);
			}
			clean_ir_sexpr(sexpr->local_scope.value);
			break;
		case CURLY_IR_TAGS_IF:
			clean_ir_sexpr(sexpr->if_expr.cond);
			clean_ir_sexpr(sexpr->if_expr.then);
			clean_ir_sexpr(sexpr->if_expr.elsy);
			break;
		default:
			break;
	}

	free(sexpr);
}

// clean_ir(curly_ir_t) -> void
// Cleans up Curly IR.
void clean_ir(curly_ir_t ir)
{
	for (size_t i = 0; i < ir.expr_count; i++)
	{
		clean_ir_sexpr(ir.expr[i]);
	}

	free(ir.expr);
}

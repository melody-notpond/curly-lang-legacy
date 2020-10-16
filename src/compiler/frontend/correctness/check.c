// 
// correctness
// check.c: Checks if an ast is semantically valid.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#include <stdio.h>
#include <string.h>

#include "../../../utils/list.h"
#include "check.h"
#include "type_generators.h"

// check_correctness_helper(ast_t*, ir_scope_t* /*, bool, bool*/) -> void
// Helper function for check_correctness.
bool check_correctness_helper(ir_sexpr_t* sexpr, ir_scope_t* scope /*, bool get_real_type, bool disable_new_vars*/)
{
	// Match the sum type
	switch (sexpr->tag)
	{
		case CURLY_IR_TAGS_INT:
			sexpr->type = scope_lookup_type(scope, "Int");
			return true;
		case CURLY_IR_TAGS_FLOAT:
			sexpr->type = scope_lookup_type(scope, "Float");
			return true;
		case CURLY_IR_TAGS_BOOL:
			sexpr->type = scope_lookup_type(scope, "Bool");
			return true;
		case CURLY_IR_TAGS_SYMBOL:
			// Get type from variable list
			sexpr->type = scope_lookup_var_type(scope, sexpr->symbol);

			// If variable isn't found, report an error
			if (sexpr->type == NULL)
				printf("Undeclared variable %s found at %i:%i\n", sexpr->symbol, sexpr->lino, sexpr->charpos);
			return sexpr->type != NULL;

		case CURLY_IR_TAGS_ASSIGN:
			// Check value
			if (!check_correctness_helper(sexpr->assign.value, scope))
				return false;

			// Check type doesn't change on reassignment
			type_t* type = map_get(scope->var_types, sexpr->assign.name);
			if (type != NULL && !type_subtype(type, sexpr->assign.value->type))
			{
				printf("Assigning incompatible type to %s found at %i:%i\n", sexpr->assign.name, sexpr->lino, sexpr->charpos);
				return false;
			} else sexpr->type = type;

			// Assert the type is empty or matches
			if (sexpr->type == NULL)
				sexpr->type = sexpr->assign.value->type;
			else if (!type_subtype(sexpr->type, sexpr->assign.value->type))
			{
				printf("Assigning incompatible type to %s found at %i:%i\n", sexpr->assign.name, sexpr->lino, sexpr->charpos);
				return false;
			}

			// Add the value and return
			map_add(scope->var_types, sexpr->assign.name, sexpr->type);
			map_add(scope->var_vals, sexpr->assign.name, sexpr->assign.value);
			return true;

		case CURLY_IR_TAGS_DECLARE:
			map_add(scope->var_types, sexpr->declare.name, sexpr->type);
			return true;

		case CURLY_IR_TAGS_LOCAL_SCOPE:
			// Create a new scope
			scope = push_scope(scope);

			// Deal with assignments
			for (size_t i = 0; i < sexpr->local_scope.assign_count; i++)
			{
				if (!check_correctness_helper(sexpr->local_scope.assigns[i], scope))
					return false;
			}

			// Deal with the expression
			if (!check_correctness_helper(sexpr->local_scope.value, scope))
				return false;

			// Pop scope and set the type
			scope = pop_scope(scope);
			sexpr->type = sexpr->local_scope.value->type;
			return true;

		case CURLY_IR_TAGS_IF:
			// Check that the condition is a boolean
			if (!check_correctness_helper(sexpr->if_expr.cond, scope))
				return false;
			if (!types_equal(sexpr->if_expr.cond->type, scope_lookup_type(scope, "Bool")))
			{
				printf("Nonboolean condition found at %i:%i\n", sexpr->if_expr.cond->lino, sexpr->if_expr.cond->charpos);
				return false;
			}

			// Check the then clause
			if (!check_correctness_helper(sexpr->if_expr.then, scope))
				return false;
			sexpr->type = sexpr->if_expr.then->type;

			// Check the else clause
			if (!check_correctness_helper(sexpr->if_expr.elsy, scope))
				return false;

			// Else clause should have the same type as the body
			if (!types_equal(sexpr->type, sexpr->if_expr.elsy->type))
			{
				printf("If statement with different types for bodies at %i:%i\n", sexpr->lino, sexpr->charpos);
				return false;
			}

			// Return successfully
			return true;

		case CURLY_IR_TAGS_INFIX:
			switch (sexpr->infix.op)
			{
				case IR_BINOPS_CMPIN:
					// Check the operands
					if (!check_correctness_helper(sexpr->infix.right, scope))
						return false;

					// Assert that the second operand is an iterator
					if (sexpr->infix.right->type->type_type != IR_TYPES_LIST && sexpr->infix.right->type->type_type != IR_TYPES_GENERATOR)
					{
						printf("Noniterator used in in expression found at %i:%i\n", sexpr->infix.right->lino, sexpr->infix.right->charpos);
						return false;
					}

					// Assert the type makes sense
					if (!types_equal(sexpr->infix.left->type, sexpr->infix.right->type->field_types[0]))
					{
						printf("Mismatched types found at %i:%i", sexpr->lino, sexpr->charpos);
						return false;
					}
					sexpr->type = scope_lookup_type(scope, "Bool");
					return true;

				case IR_BINOPS_CMPEQ:
				case IR_BINOPS_CMPNEQ:
				case IR_BINOPS_CMPGT:
				case IR_BINOPS_CMPGTE:
				case IR_BINOPS_CMPLT:
				case IR_BINOPS_CMPLTE:
					// Check the operands
					if (!check_correctness_helper(sexpr->infix.left, scope))
						return false;
					if (!check_correctness_helper(sexpr->infix.right, scope))
						return false;

					// Set the type of the s expression to bool and return success
					sexpr->type = scope_lookup_type(scope, "Bool");
					return true;

				case IR_BINOPS_BOOLAND:
				case IR_BINOPS_BOOLOR:
				case IR_BINOPS_BOOLXOR:
				{
					// Check the operands
					if (!check_correctness_helper(sexpr->infix.left, scope))
						return false;
					if (!check_correctness_helper(sexpr->infix.right, scope))
						return false;

					// Assert that both operands are booleans
					type_t* boolean = scope_lookup_type(scope, "Bool");
					if (!types_equal(sexpr->infix.left->type, boolean))
					{
						printf("Nonboolean used for logical expression found at %i:%i\n", sexpr->infix.left->lino, sexpr->infix.left->charpos);
						return false;
					} else if (!types_equal(sexpr->infix.right->type, boolean))
					{
						printf("Nonboolean used for logical expression found at %i:%i\n", sexpr->infix.right->lino, sexpr->infix.right->charpos);
						return false;
					}

					// Set the type and return success
					sexpr->type = boolean;
					return true;
				}

				default:
					// Check the operands
					if (!check_correctness_helper(sexpr->infix.left, scope))
						return false;
					if (!check_correctness_helper(sexpr->infix.right, scope))
						return false;

					// Find the type of the resulting infix expression
					sexpr->type = scope_lookup_infix(scope, sexpr->infix.op, sexpr->infix.left->type, sexpr->infix.right->type);
					if (sexpr->type == NULL)
					{
						printf("Undefined infix operator found at %i:%i\n", sexpr->lino, sexpr->charpos);
						return false;
					}
					return true;
			}

		case CURLY_IR_TAGS_PREFIX:
			switch (sexpr->prefix.op)
			{
				case IR_BINOPS_SPAN:
					// Check the child node
					if (!check_correctness_helper(sexpr->prefix.operand, scope))
						return false;

					// Assert the type is a list or generator
					if (sexpr->prefix.operand->type->type_type != IR_TYPES_PRODUCT)
					{
						printf("Curry on invalid operand found at %i:%i\n", sexpr->lino, sexpr->charpos);
						return false;
					}

					// Create the type and return success
					sexpr->type = init_type(IR_TYPES_CURRY, NULL, sexpr->prefix.operand->type->field_count);
					for (size_t i = 0; i < sexpr->type->field_count; i++)
					{
						sexpr->type->field_types[i] = sexpr->prefix.operand->type->field_types[i];
					}
					return true;
				case IR_BINOPS_NEG:
					// Check the operand
					if (!check_correctness_helper(sexpr->prefix.operand, scope))
						return false;

					// Find the type of the resulting prefix expression
					sexpr->type = scope_lookup_prefix(scope, sexpr->prefix.operand->type);
					if (sexpr->type == NULL)
					{
						printf("Negative sign on invalid operand found at %i:%i\n", sexpr->lino, sexpr->charpos);
						return false;
					}
					return true;
				default:
					printf("Unsupported prefix operator found at %i:%i\n", sexpr->lino, sexpr->charpos);
					return false;
			}

		default:
			printf("Unsupported s expression found at %i:%i\n", sexpr->lino, sexpr->charpos);
			return false;
	}
}

// check_correctness(curly_ir_t, ir_scope_t*) -> void
// Checks the correctness of IR.
bool check_correctness(curly_ir_t ir, ir_scope_t* scope)
{
	// Create global scope
	bool temp_scope = scope == NULL;
	if (temp_scope)
		scope = push_scope(NULL);
	create_primatives(scope);

	// Iterate over every root S expression
	for (size_t i = 0; i < ir.expr_count; i++)
	{
		// Check the S expression
		if (!check_correctness_helper(ir.expr[i], scope))
		{
			// Pop scopes if failed
			while (scope->parent != NULL)
			{
				scope = pop_scope(scope);
			}
			if (temp_scope)
				pop_scope(scope);
			return false;
		}
	}

	// Pop scopes and return success
	if (temp_scope)
		pop_scope(scope);
	return true;
}

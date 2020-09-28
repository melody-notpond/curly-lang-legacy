// 
// correctness
// check_assign.c: Checks an assignment.
// 
// Created by jenra.
// Created on September 5 2020.
// 

#include <stdio.h>
#include <string.h>

#include "check_helper.h"
#include "check_assign.h"
#include "type_generators.h"

// check_assign(ast_t*, ir_scope_t*, bool, bool) -> bool
// Checks if an assignment is valid.
bool check_assign(ast_t* ast, ir_scope_t* scope, bool get_real_type, bool disable_new_vars)
{
	// var = expr (cannot be recursive)
	if (ast->children[0]->value.type == LEX_TYPE_SYMBOL && ast->children[0]->children_count == 0)
	{
		// Get the type of the value
		ast_t* var_ast = ast->children[0];
		ast_t* val_ast = ast->children[1];
		if (!check_correctness_helper(val_ast, scope, get_real_type, disable_new_vars)) return false;
		type_t* type = val_ast->type;

		// Add the type if it does not exist
		type_t* var_type = scope_lookup_var_type(scope, var_ast->value.value);
		if (var_type == NULL)
		{
			// Disable new variables if necessary
			if (disable_new_vars)
			{
				printf("Unable to create new variables in current context at %i:%i\n", var_ast->value.lino, var_ast->value.charpos);
				return false;
			}

			var_ast->type = type;
			ast->type = type;

			// Deal with adding new types
			if (types_equal(var_ast->type, scope_lookup_type(scope, "Type")))
			{
				// Mutable types are not allowed
				if (map_contains(scope->types, var_ast->value.value))
				{
					printf("Mutable type %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
					return false;
				} else
				{
					// Create and save type
					type_t* type = generate_type(val_ast, scope, var_ast, NULL);
					if (type == NULL) return false;
					type->type_name = strdup(var_ast->value.value);
					map_add(scope->types, var_ast->value.value, type);
					return true;
				}
			} else
			{
				// Save variable type and value
				map_add(scope->var_types, var_ast->value.value, type);
				map_add(scope->var_vals, var_ast->value.value, val_ast);
				return true;
			}

			return true;

		// Check if the type is correct
		} else if (type_subtype(var_type, type, true))
		{
			map_add(scope->var_types, var_ast->value.value, type);
			map_add(scope->var_vals, var_ast->value.value, val_ast);
			ast->type = type;
			return true;
		}

		// Error since the types are not equal
		else
		{
			printf("Assigning incompatible type to %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
			return false;
		}

	// var: type = expr (can be recursive)
	} else if (ast->children[0]->value.type == LEX_TYPE_COLON)
	{
		// Disable new variables if necessary
		ast_t* var_ast = ast->children[0]->children[0];
		if (disable_new_vars)
		{
			printf("Unable to create new variables in current context at %i:%i\n", var_ast->value.lino, var_ast->value.charpos);
			return false;
		}

		// Get the type and var name ast
		ast_t* type_ast = ast->children[0]->children[1];
		type_ast->type = scope_lookup_type(scope, "Type");

		// Check for redeclarations
		if (map_contains(scope->var_types, var_ast->value.value))
		{
			printf("Redeclaration of %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
			return false;
		}

		// Construct a type
		type_t* type = generate_type(type_ast, scope, NULL, NULL);
		if (type == NULL) return false;
		var_ast->type = type;
		ast->type = type;

		// Add variable type to the scope
		map_add(scope->var_types, var_ast->value.value, type);

		// Get the type of the expression
		ast_t* val_ast = ast->children[1];

		// Check for self assignment (var: type = var)
		if (val_ast->value.type == LEX_TYPE_SYMBOL && !strcmp(var_ast->value.value, val_ast->value.value))
		{
			printf("Self assignment of %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
			return false;
		}

		// Deal with adding new types
		if (types_equal(type, scope_lookup_type(scope, "Type")))
		{
			// Error on reassigning a previously defined type
			if (map_contains(scope->types, var_ast->value.value))
			{
				printf("Mutable type %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
				return false;
			} else
			{
				// Add the new type
				type_t* type = generate_type(val_ast, scope, var_ast, NULL);
				if (type == NULL) return false;
				type->type_name = strdup(var_ast->value.value);
				map_add(scope->types, var_ast->value.value, type);
				return true;
			}

		// Deal with adding new enums
		} else if (types_equal(type, scope_lookup_type(scope, "Enum")))
		{
			// Error on reassigning a previously defined enum
			if (map_contains(scope->types, var_ast->value.value))
			{
				printf("Mutable enum %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
				return false;
			} else
			{
				// Add the new enum
				type_t* type = generate_enum(val_ast, scope, NULL);
				if (type == NULL) return false;
				type->type_name = strdup(var_ast->value.value);
				map_add(scope->types, var_ast->value.value, type);
				return true;
			}

		// Variable is not a type
		} else
		{
			// Assert the type of the value and variable are the same
			if (!check_correctness_helper(val_ast, scope, get_real_type, disable_new_vars)) return false;
			if (!type_subtype(var_ast->type, val_ast->type, true))
			{
				printf("Types do not match at %i:%i - %i:%i\n", var_ast->value.lino, var_ast->value.charpos, val_ast->value.lino, val_ast->value.charpos);
				return false;
			}

			// Add variable value to the scope
			map_add(scope->var_vals, var_ast->value.value, val_ast);
			return true;
		}

	// x..xs = expr (cannot be recursive)
	} else if (ast->children[0]->value.type == LEX_TYPE_RANGE)
	{
		// Check if new variables are disabled
		type_t* head_type = scope_lookup_var_type(scope, ast->children[0]->children[0]->value.value);
		type_t* tail_type = scope_lookup_var_type(scope, ast->children[0]->children[1]->value.value);
		if (disable_new_vars && (head_type == NULL || tail_type == NULL))
		{
			printf("Unable to create new variables in current context at %i:%i\n", ast->children[0]->value.lino, ast->children[0]->value.charpos);
			return false;
		}

		// Check expression
		if (!check_correctness_helper(ast->children[1], scope, get_real_type, disable_new_vars))
			return false;

		// Assert that it's an iterator
		if (ast->children[1]->type->type_type != IR_TYPES_LIST && ast->children[1]->type->type_type != IR_TYPES_GENERATOR)
		{
			printf("Range assignment of noniterator found at %i:%i\n", ast->children[1]->value.lino, ast->children[1]->value.charpos);
			return false;
		}

		// Set types and return success
		ast_t* head = ast->children[0]->children[0];
		ast_t* tail = ast->children[0]->children[1];
		head->type = ast->children[1]->type->field_types[0];
		tail->type = ast->children[1]->type;
		ast->type = ast->children[1]->type->field_types[0];

		// Check type if head existed before
		type_t* type = scope_lookup_var_type(scope, head->value.value);
		if (type == NULL)
		{
			map_add(scope->var_types, head->value.value, head->type);
			map_add(scope->var_vals, head->value.value, head);
		} else if (!type_subtype(type, head->type, true))
		{
			printf("Mismatched types found at %i:%i\n", head->value.lino, head->value.charpos);
			return false;
		}

		// Check type if tail existed before
		type = scope_lookup_var_type(scope, tail->value.value);
		if (type == NULL)
		{
			map_add(scope->var_types, tail->value.value, tail->type);
			map_add(scope->var_vals, tail->value.value, tail);
		} else if (!type_subtype(type, tail->type, true))
		{
			printf("Mismatched types found at %i:%i\n", tail->value.lino, tail->value.charpos);
			return false;
		}

		// Return success
		return true;

	// func (arg: type | value)* = expr (can be recursive)
	} else if (ast->children[0]->value.type == LEX_TYPE_SYMBOL)
	{
		// Check if new variables are disabled
		type_t* var_type = scope_lookup_var_type(scope, ast->children[0]->value.value);
		if (disable_new_vars && var_type == NULL)
		{
			printf("Unable to create new variables in current context at %i:%i\n", ast->children[0]->value.lino, ast->children[0]->value.charpos);
			return false;
		}

		// Push a new scope
		scope = push_scope(scope);

		// Create the type of the function
		type_t* type = init_type(IR_TYPES_FUNC, NULL, 2);
		type_t* tail = type;
		if (!check_correctness_helper(ast->children[0]->children[0], scope, get_real_type, disable_new_vars))
			return false;
		tail->field_types[0] = ast->children[0]->children[0]->type;

		// Create all the arguments
		for (size_t i = 1; i < ast->children[0]->children_count; i++)
		{
			tail->field_types[1] = init_type(IR_TYPES_FUNC, NULL, 2);
			tail = tail->field_types[1];
			if (!check_correctness_helper(ast->children[0]->children[i], scope, get_real_type, disable_new_vars))
				return false;
			tail->field_types[0] = ast->children[0]->children[i]->type;
		}

		// Check the body
		if (!check_correctness_helper(ast->children[1], scope, get_real_type, disable_new_vars))
			return false;
		tail->field_types[1] = ast->children[1]->type;

		// Pop the scope
		scope = pop_scope(scope);

		// If previously defined, check that the types match
		if (var_type != NULL && !type_subtype(var_type, type, true))
		{
			printf("Mismatched types found at %i:%i\n", ast->children[0]->value.lino, ast->children[0]->value.charpos);
			return false;

		// Add the variable if it's new
		} else if (var_type == NULL)
		{
			map_add(scope->var_types, ast->children[0]->value.value, type);
			map_add(scope->var_vals, ast->children[0]->value.value, ast->children[1]);
		}

		// Set types and return success
		ast->children[0]->type = type;
		ast->type = type;
		print_type(type);
		return true;

	// TODO: attribute assignment
	} else return false;
}

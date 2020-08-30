// 
// correctness
// check.c: Checks if an ast is semantically valid.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#include <stdio.h>
#include <string.h>

#include "check.h"

// check_correctness_helper(ast_t*, ir_scope_t*) -> void
// Helper function for check_correctness.
bool check_correctness_helper(ast_t* ast, ir_scope_t* scope)
{
	// Leaf node of ast
	if (ast->value.tag == LEX_TAG_OPERAND)
	{
		// Set primitive type
		switch (ast->value.type)
		{
			case LEX_TYPE_INT:
				ast->type = scope_lookup_type(scope, "int");
				return true;
			case LEX_TYPE_FLOAT:
				ast->type = scope_lookup_type(scope, "float");
				return true;
			case LEX_TYPE_STRING:
				ast->type = scope_lookup_type(scope, "string");
				return true;
			case LEX_TYPE_BOOL:
				ast->type = scope_lookup_type(scope, "bool");
				return true;
			case LEX_TYPE_NIL:
				ast->type = scope_lookup_type(scope, "obj");
				return true;
			case LEX_TYPE_SYMBOL:
				// Get type from variable list
				ast->type = scope_lookup_var_type(scope, ast->value.value);

				// If variable isn't found, try finding it as a type
				if (ast->type == NULL && scope_lookup_type(scope, ast->value.value) != NULL)
					ast->type = scope_lookup_type(scope, "type");

				// If variable isn't found, report an error
				else
					printf("Undeclared variable %s found at %i:%i\n", ast->value.value, ast->value.lino, ast->value.charpos);
				return ast->type != NULL;
			default:
				// Unknown type
				printf("Unknown type found at %i:%i\n", ast->value.lino, ast->value.charpos);
				return false;
		}

	// Assign nodes with undeclared variables
	} else if (ast->value.type == LEX_TYPE_ASSIGN)
	{
		// var (arg: type)* = expr
		if (ast->children[0]->value.type == LEX_TYPE_SYMBOL)
		{
			// Get the type of the value
			ast_t* var_ast = ast->children[0];
			ast_t* val_ast = ast->children[1];
			if (!check_correctness_helper(val_ast, scope)) return false;
			type_t* type = val_ast->type;

			// var = expr (cannot be recursive)
			if (var_ast->children_count == 0)
			{
				// Add the type if it does not exist
				type_t* var_type = scope_lookup_var_type(scope, var_ast->value.value);
				if (var_type == NULL)
				{
					var_ast->type = type;
					return true;

				// Check if the type is correct
				} else if (types_equal(var_type, type))
					return true;

				// Error since the types are not equal
				else
				{
					printf("Assigning incompatible type to %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
					return false;
				}
			} else return false;

		// var: type = expr (can be recursive)
		} else if (ast->children[0]->value.type == LEX_TYPE_COLON)
		{
			// Get the type and var name ast
			ast_t* type_ast = ast->children[0]->children[1];
			type_ast->type = scope_lookup_type(scope, "type");
			ast_t* var_ast = ast->children[0]->children[0];

			if (map_contains(scope->var_types, var_ast->value.value))
			{
				printf("Redeclaration of %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
				return false;
			}

			// Deal with symbol types
			if (type_ast->value.type == LEX_TYPE_SYMBOL && type_ast->children_count == 0)
			{
				type_t* type = scope_lookup_type(scope, type_ast->value.value);
				if (type == NULL)
				{
					printf("Undeclared type %s found at %i:%i\n", type_ast->value.value, type_ast->value.lino, type_ast->value.charpos);
					return false;
				}
				var_ast->type = type;

			// Construct a type
			} else if (type_ast->value.type == LEX_TYPE_SYMBOL || type_ast->value.tag == LEX_TAG_INFIX_OPERATOR)
			{
				// TODO
			}

			// Add variable type to the scope
			map_add(scope->var_types, var_ast->value.value, var_ast->type);

			// Get the type of the expression
			ast_t* val_ast = ast->children[1];
			if (!check_correctness_helper(val_ast, scope)) return false;

			// Assert the type of the value and variable are the same
			if (!types_equal(var_ast->type, val_ast->type))
			{
				printf("Types do not match at %i:%i - %i:%i\n", var_ast->value.lino, var_ast->value.charpos, val_ast->value.lino, val_ast->value.charpos);
				return false;
			}

			// Check for self assignment (var: type = var)
			if (val_ast->value.type == LEX_TYPE_SYMBOL && !strcmp(var_ast->value.value, val_ast->value.value))
			{
				printf("Self assignment of %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
				return false;
			}

			// Add variable value to the scope
			map_add(scope->var_vals, var_ast->value.value, val_ast);
			return true;

		// TODO: range and function assignment
		} else return false;

	// Lists
	} else if (!strcmp(ast->value.value, "["))
		{
			// Empty list type for empty lists
			if (ast->children_count == 0)
			{
				ast->type = init_type(IR_TYPES_LIST, NULL, ast->children_count > 0);
				return true;
			}

			// Create type list
			if (!check_correctness_helper(ast->children[0], scope)) return false;
			if (ast->children_count == 0 && types_equal(ast->children[0]->type, scope_lookup_type(scope, "type")))
			{
				ast->type = ast->children[0]->type;
				return true;
			}

			// Check that the type of the first element is the same as the type of the rest of the elements
			type_t* elem_type = ast->children[0]->type;
			for (int i = 1; i < ast->children_count; i++)
			{
				if (!check_correctness_helper(ast->children[i], scope)) return false;
				if (!types_equal(elem_type, ast->children[i]->type))
				{
					// Report error
					printf("List has different types at %i:%i and %i:%i\n", ast->children[0]->value.lino, ast->children[0]->value.charpos, ast->children[i]->value.lino, ast->children[i]->value.charpos);
					return false;
				}
			}

			// Create the list type
			ast->type = init_type(IR_TYPES_LIST, NULL, 1);
			ast->type->field_types[0] = elem_type;
			return true;

		// TODO: literally everything else
		} else return false;
	return false;
}

// check_correctness(ast_t*) -> void
// Checks the correctness of an ast.
bool check_correctness(ast_t* ast)
{
	// Create global scope
	ir_scope_t* scope = push_scope(NULL);

	// Add primitives to the global scope
	create_primatives();
	type_t* head = type_linked_list_head;
	while (head != NULL)
	{
		map_add(scope->types, head->type_name, head);
		head = head->next;
	}

	// Check the correctness of child nodes if top node
	if (ast->value.type == LEX_TYPE_NONE)
	{
		for (int i = 0; i < ast->children_count; i++)
		{
			if (!check_correctness_helper(ast->children[i], scope))
				return false;
		}
		return true;

	// Check the correctness of the node itself
	} else return check_correctness_helper(ast, scope);
}

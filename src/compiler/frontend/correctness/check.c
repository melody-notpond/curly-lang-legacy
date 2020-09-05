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
#include "check_assign.h"
#include "type_generators.h"

// check_correctness_helper(ast_t*, ir_scope_t*, bool, bool) -> void
// Helper function for check_correctness.
bool check_correctness_helper(ast_t* ast, ir_scope_t* scope, bool get_real_type, bool disable_new_vars)
{
	// Leaf node of ast
	if (ast->value.tag == LEX_TAG_OPERAND)
	{
		// Set primitive type
		switch (ast->value.type)
		{
			case LEX_TYPE_INT:
				ast->type = scope_lookup_type(scope, "Int");
				return true;
			case LEX_TYPE_FLOAT:
				ast->type = scope_lookup_type(scope, "Float");
				return true;
			case LEX_TYPE_STRING:
				ast->type = scope_lookup_type(scope, "String");
				return true;
			case LEX_TYPE_BOOL:
				ast->type = scope_lookup_type(scope, "Bool");
				return true;
			case LEX_TYPE_SYMBOL:
			{
				// Get type from variable list
				ast_t* type_ast = scope_lookup_var_val(scope, ast->value.value);
				ast->type = get_real_type && type_ast != NULL ? type_ast->type : scope_lookup_var_type(scope, ast->value.value);

				// If variable isn't found, try finding it as a type
				if (ast->type == NULL && scope_lookup_type(scope, ast->value.value) != NULL)
					ast->type = scope_lookup_type(scope, "Type");

				// If variable isn't found, report an error
				if (ast->type == NULL)
					printf("Undeclared variable %s found at %i:%i\n", ast->value.value, ast->value.lino, ast->value.charpos);
				return ast->type != NULL;
			}
			default:
				// Unknown type
				printf("Unknown type found at %i:%i\n", ast->value.lino, ast->value.charpos);
				return false;
		}

	// Function applications
	} else if (ast->value.type == LEX_TYPE_APPLICATION)
	{
		// Check the applicator and argument
		if (!check_correctness_helper(ast->children[0], scope, get_real_type, disable_new_vars))
			return false;
		if (!check_correctness_helper(ast->children[1], scope, get_real_type, disable_new_vars))
			return false;

		// If it's a type, then apply the type
		if (types_equal(ast->children[0]->type, scope_lookup_type(scope, "Type")))
		{
			// Get the type
			type_t* type;
			if (ast->children[0]->value.type == LEX_TYPE_SYMBOL)
				type = scope_lookup_type(scope, ast->children[0]->value.value);
			else
			{
				printf("Parametric types are currently unsupported.\n");
				return false;
			}

			// Check if the argument can be converted into the type
			if (!type_subtype(type, ast->children[1]->type, true))
			{
				printf("Casting to incompatible type at %i:%i\n", ast->children[1]->value.lino, ast->children[1]->value.charpos);
				return false;
			}

			// Set the type and return success
			ast->type = type;
			return true;

		// Function application
		} else if (ast->children[0]->type->type_type == IR_TYPES_FUNC)
		{
			// Check the argument type
			type_t* func_type = ast->children[0]->type;
			type_t* arg_type = ast->children[1]->type;
			if (arg_type->type_type == IR_TYPES_CURRY)
			{
				// Check the subtypes of the curry
				for (size_t i = 0; i < arg_type->field_count; i++)
				{
					if (func_type->type_type != IR_TYPES_FUNC || !type_subtype(func_type->field_types[0], arg_type->field_types[i], true))
					{
						printf("Invalid type passed into function found at %i:%i\n", ast->value.lino, ast->value.charpos);
						return false;
					}
					func_type = func_type->field_types[1];
				}

				// Set type and return success
				ast->type = func_type;
				return true;
			} else if (!type_subtype(func_type->field_types[0], arg_type, true))
			{
				printf("Invalid type passed into function found at %i:%i\n", ast->value.lino, ast->value.charpos);
				return false;
			}

			// Set type and return success
			ast->type = func_type->field_types[1];
			return true;

		// Concatenating strings
		} else if (types_equal(ast->children[0]->type, scope_lookup_type(scope, "String")))
		{
			// Set type and return success
			ast->type = scope_lookup_type(scope, "String");
			return true;

		// Other kinds of applications are not supported at the moment
		} else
		{
			printf("Unsupported application found at %i:%i\n", ast->children[0]->value.lino, ast->children[0]->value.charpos);
			return false;
		}

	// Assign nodes with undeclared variables
	} else if (ast->value.type == LEX_TYPE_ASSIGN)
	{
		return check_assign(ast, scope, get_real_type, disable_new_vars);

	// Lists
	} else if (!strcmp(ast->value.value, "["))
	{
		// Empty list type for empty lists
		if (ast->children_count == 0)
		{
			ast->type = init_type(IR_TYPES_LIST, NULL, 0);
			return true;
		}

		// Check the first element
		if (!check_correctness_helper(ast->children[0], scope, false, disable_new_vars))
			return false;

		// Check that the type of the first element is the same as the type of the rest of the elements
		type_t* elem_type = ast->children[0]->type;
		for (int i = 1; i < ast->children_count; i++)
		{
			if (!check_correctness_helper(ast->children[i], scope, false, disable_new_vars)) return false;
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

	// Structures
	} else if (!strcmp(ast->value.value, "{"))
	{
		// Create the type
		type_t* type = init_type(IR_TYPES_PRODUCT, NULL, ast->children_count);
		for (int i = 0; i < ast->children_count; i++)
		{
			// Set the subtype
			ast_t* child = ast->children[i];
			ast_t* node = child->value.type == LEX_TYPE_ASSIGN ? child->children[1] : child;
			if (!check_correctness_helper(node, scope, get_real_type, disable_new_vars)) return false;
			type->field_types[i] = node->type;

			if (node != child)
			{
				child->type = node->type;
				child->children[0]->type = node->type;
				type->field_names[i] = strdup(child->children[0]->value.value);
			}
		}

		// Set the ast's type
		ast->type = type;
		return true;

	// Variable declarations
	} else if (!strcmp(ast->value.value, ":"))
	{
		// Check that the variable is a symbol
		ast_t* var_ast = ast->children[0];
		if (var_ast->value.type != LEX_TYPE_SYMBOL)
		{
			printf("Declaration of nonsymbol found at %i:%i\n", var_ast->value.lino, var_ast->value.charpos);
			return false;

		// Check that the variable wasn't previously declared
		} else if (map_contains(scope->var_types, var_ast->value.value))
		{
			printf("Redeclaration of %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
			return false;
		}

		// Generate the type and assign it
		type_t* type = generate_type(ast->children[1], scope, NULL, NULL);
		if (type == NULL) return false;
		var_ast->type = type;
		ast->type = type;
		print_type(type);
		map_add(scope->var_types, var_ast->value.value, type);
		return true;

	// Getting attributes
	} else if (!strcmp(ast->value.value, "."))
	{
		// Check the leftmost node first
		if (!check_correctness_helper(ast->children[0], scope, true, disable_new_vars))
				return false;

		// If it's a structure then check if the attribute is valid
		if (ast->children[0]->type->type_type == IR_TYPES_PRODUCT || ast->children[0]->type->type_type == IR_TYPES_UNION)
		{
			// Attribute must be a symbol or an integer
			if (ast->children[1]->value.type != LEX_TYPE_SYMBOL && ast->children[1]->value.type != LEX_TYPE_INT)
			{
				printf("Access of invalid attribute %s found at %i:%i\n", ast->children[1]->value.value, ast->children[1]->value.lino, ast->children[1]->value.charpos);
				return false;
			}

			// Check that the structure has the attribute
			if (ast->children[1]->value.type == LEX_TYPE_SYMBOL)
			{
				bool found = false;
				for (size_t i = 0; i < ast->children[0]->type->field_count; i++)
				{
					if (!strcmp(ast->children[0]->type->field_names[i], ast->children[1]->value.value))
					{
						// Cannot access duplicate attributes by name
						if (found)
						{
							printf("Access of duplicate attribute %s found at %i:%i\n", ast->children[1]->value.value, ast->children[1]->value.lino, ast->children[1]->value.charpos);
							return false;
						} else
						{
							found = true;
							ast->type = ast->children[0]->type->field_types[i];
							ast->children[1]->type = ast->children[0]->type->field_types[i];
							print_type(ast->type);
						}
					}
				}

				// Print error if necessary and return
				if (!found)
					printf("Access of nonexistent attribute %s found at %i:%i\n", ast->children[1]->value.value, ast->children[1]->value.lino, ast->children[1]->value.charpos);
				return found;
			} else
			{
				// Get index
				size_t index = atoll(ast->children[1]->value.value);

				// Error if the index is greater than the number of types (there's no negative numbers)
				if (index >= ast->children[0]->type->field_count)
				{
					printf("Index %zu out of bounds for type with %zu fields found at %i:%i\n", index, ast->children[0]->type->field_count, ast->children[1]->value.lino, ast->children[1]->value.charpos);
					return false;
				}

				// Get the type
				ast->type = ast->children[0]->type->field_types[index];
				ast->children[1]->type = ast->children[0]->type->field_types[index];
				return true;
			}

		// Getting attributes from lists
		} else if (ast->children[0]->type->type_type == IR_TYPES_LIST)
		{
			// Check attribute
			if (!check_correctness_helper(ast->children[1], scope, get_real_type, disable_new_vars))
				return false;

			// Assert that the attribute is either an integer, a list of integers, or a generator of integers
			type_t* int_t = scope_lookup_type(scope, "Int");
			if (!(ast->children[1]->type->type_type == IR_TYPES_GENERATOR && types_equal(ast->children[1]->type->field_types[0], int_t))
			 && !(ast->children[1]->type->type_type == IR_TYPES_LIST      && types_equal(ast->children[1]->type->field_types[0], int_t))
			 && !types_equal(ast->children[1]->type, int_t))
			{
				printf("Invalid index for list found at %i:%i\n", ast->children[1]->value.lino, ast->children[1]->value.charpos);
				return false;
			}

			// Return success
			ast->type = ast->children[0]->type->field_types[0];
			return true;

		// Getting attributes from strings
		} else if (types_equal(ast->children[0]->type, scope_lookup_type(scope, "String")))
		{
			// Check attribute
			if (!check_correctness_helper(ast->children[1], scope, get_real_type, disable_new_vars))
				return false;

			// Assert that the attribute is either an integer, a list of integers, or a generator of integers
			type_t* int_t = scope_lookup_type(scope, "Int");
			if (!(ast->children[1]->type->type_type == IR_TYPES_GENERATOR && types_equal(ast->children[1]->type->field_types[0], int_t))
			 && !(ast->children[1]->type->type_type == IR_TYPES_LIST      && types_equal(ast->children[1]->type->field_types[0], int_t))
			 && !types_equal(ast->children[1]->type, int_t))
			{
				printf("Invalid index for string found at %i:%i\n", ast->children[1]->value.lino, ast->children[1]->value.charpos);
				return false;
			}

			// Return success
			ast->type = scope_lookup_type(scope, "String");
			return true;

		// Error on anything else
		} else
		{
			printf("Attempt at retrieving attribute from invalid object found at %i:%i\n", ast->value.lino, ast->value.charpos);
			return false;
		}

	// with expressions
	} else if (!strcmp(ast->value.value, "with"))
	{
		// Create a new scope
		scope = push_scope(scope);

		// Deal with child nodes
		for (size_t i = 0; i < ast->children_count; i++)
		{
			if (!check_correctness_helper(ast->children[i], scope, get_real_type, false))
				return false;
		}

		// Pop scope and set the type
		scope = pop_scope(scope);
		ast->type = ast->children[ast->children_count - 1]->type;
		return true;

	// For loops
	} else if (!strcmp(ast->value.value, "for"))
	{
		// Check the second node (the iterator)
		ast_t* iterator = ast->children[ast->children_count - 2];
		ast_t* var = ast->children[ast->children_count - 3];
		if (!check_correctness_helper(iterator, scope, get_real_type, true))
			return false;

		// The second node must be a list or generator type
		if (iterator->type->type_type != IR_TYPES_LIST && iterator->type->type_type != IR_TYPES_GENERATOR)
		{
			printf("Attempted iteration over noniterator type at %i:%i\n", ast->children[1]->value.lino, ast->children[1]->value.charpos);
			return false;
		}

		// Register the iterating symbol as a variable
		var->type = iterator->type->field_types[0];
		map_add(scope->var_types, var->value.value, iterator->type->field_types[0]);

		// Check the body of the for loop
		ast_t* body = ast->children[ast->children_count - 1];
		if (!check_correctness_helper(body, scope, get_real_type, true))
			return false;

		// Remove the iterator variable
		map_remove(scope->var_types, var->value.value);

		// If there is some quantifier (pun intended), make sure the type of the body is booleans
		if (ast->children_count == 4)
		{
			if (!types_equal(body->type, scope_lookup_type(scope, "Bool")))
			{
				printf("Quantifier returns something other than a boolean found at %i:%i\n", ast->value.lino, ast->value.charpos);
				return false;
			}

			// Return success
			ast->type = body->type;
			return true;
		}

		// Update the type of the loop and return success
		ast->type = init_type(IR_TYPES_GENERATOR, NULL, 1);
		ast->type->field_types[0] = body->type;
		print_type(ast->type);
		return true;

	// If expressions
	} else if (!strcmp(ast->value.value, "if"))
	{
		// Check that the condition is a boolean
		if (!check_correctness_helper(ast->children[0], scope, get_real_type, disable_new_vars))
			return false;
		if (!types_equal(ast->children[0]->type, scope_lookup_type(scope, "Bool")))
		{
			printf("Nonboolean condition found at %i:%i\n", ast->children[0]->value.lino, ast->children[0]->value.charpos);
			return false;
		}

		// Check the body
		if (!check_correctness_helper(ast->children[1], scope, get_real_type, true))
			return false;
		ast->type = ast->children[1]->type;

		// If (pun intended) there's an else clause, deal with that
		if (ast->children_count == 3)
		{
			if (!check_correctness_helper(ast->children[2], scope, get_real_type, true))
				return false;

			// Else clause should have the same type as the body
			if (!types_equal(ast->type, ast->children[2]->type))
			{
				printf("If statement with different types for bodies at %i:%i\n", ast->value.lino, ast->value.charpos);
				return false;
			}
		}

		// Return successfully
		return true;

	// Where expressions
	} else if (!strcmp(ast->value.value, "where"))
	{
		// Check the first node
		if (!check_correctness_helper(ast->children[0], scope, get_real_type, disable_new_vars))
			return false;

		// Assert the second node is a boolean type
		if (!check_correctness_helper(ast->children[1], scope, get_real_type, disable_new_vars))
			return false;
		if (!types_equal(ast->children[1]->type, scope_lookup_type(scope, "Bool")))
		{
			printf("Where expression with nonboolean type in body found at %i:%i\n", ast->children[1]->value.lino, ast->children[1]->value.charpos);
			return false;
		}

		// Set the type and return success
		ast->type = init_type(IR_TYPES_GENERATOR, NULL, 1);
		ast->type->field_types[0] = ast->children[0]->children[0]->type;
		return true;

	// In operator
	} else if (!strcmp(ast->value.value, "in"))
	{
		// Check the second node
		if (!check_correctness_helper(ast->children[1], scope, get_real_type, disable_new_vars))
			return false;

		// Assert that the second type is an iterator
		if (ast->children[1]->type->type_type != IR_TYPES_LIST && ast->children[1]->type->type_type != IR_TYPES_GENERATOR)
		{
			printf("Noniterator used in in expression found at %i:%i\n", ast->children[1]->value.lino, ast->children[1]->value.charpos);
			return false;
		}

		// Set types and return success
		ast->children[0]->type = ast->children[1]->type->field_types[0];
		ast->type = scope_lookup_type(scope, "Bool");
		return true;

	// Comparison operators
	} else if (ast->value.type == LEX_TYPE_COMPARE)
	{
		// Check both child nodes
		if (!check_correctness_helper(ast->children[0], scope, get_real_type, disable_new_vars))
			return false;
		if (!check_correctness_helper(ast->children[1], scope, get_real_type, disable_new_vars))
			return false;

		// Set the type of the node to bool and return success
		ast->type = scope_lookup_type(scope, "Bool");
		return true;

	// and, or, and xor operators
	} else if (!strcmp(ast->value.value, "and") || !strcmp(ast->value.value, "or") || !strcmp(ast->value.value, "xor"))
	{
		// Check both child nodes
		if (!check_correctness_helper(ast->children[0], scope, get_real_type, disable_new_vars))
			return false;
		if (!check_correctness_helper(ast->children[1], scope, get_real_type, disable_new_vars))
			return false;

		// Assert that both nodes are booleans
		int i = 0;
		if (!types_equal(ast->children[0]->type, scope_lookup_type(scope, "Bool")) || !types_equal(ast->children[++i]->type, scope_lookup_type(scope, "Bool")))
		{
			printf("Nonboolean used for and expression found at %i:%i\n", ast->children[i]->value.lino, ast->children[i]->value.charpos);
			return false;
		}

		// Set the type and return success
		ast->type = scope_lookup_type(scope, "Bool");
		return true;

	// Infix operators
	} else if (ast->value.tag == LEX_TAG_INFIX_OPERATOR)
	{
		// Check the child nodes and adjust the type of declarations
		if (!check_correctness_helper(ast->children[0], scope, get_real_type, disable_new_vars))
			return false;
		if (ast->children[0]->value.type == LEX_TYPE_COLON)
			ast->children[0]->type = scope_lookup_type(scope, "Type");
		if (!check_correctness_helper(ast->children[1], scope, get_real_type, disable_new_vars))
			return false;
		if (ast->children[1]->value.type == LEX_TYPE_COLON)
			ast->children[1]->type = scope_lookup_type(scope, "Type");

		// Find the type of the resulting infix expression
		type_t* type = scope_lookup_infix(scope, ast->value.value, ast->children[0]->type, ast->children[1]->type);
		if (type == NULL)
		{
			printf("Undefined infix operator found at %i:%i\n", ast->value.lino, ast->value.charpos);
			return false;
		}

		// Set type and return success
		ast->type = type;
		return true;

	// Currying
	} else if (!strcmp(ast->value.value, "*"))
	{
		// Check the child node
		if (!check_correctness_helper(ast->children[0], scope, get_real_type, disable_new_vars))
			return false;

		// Assert the type is a list or generator
		if (ast->children[0]->type->type_type != IR_TYPES_PRODUCT)
		{
			printf("Curry on invalid operand found at %i:%i\n", ast->children[0]->value.lino, ast->children[0]->value.charpos);
			return false;
		}

		// Create the type and return success
		ast->type = init_type(IR_TYPES_CURRY, NULL, ast->children[0]->type->field_count);
		for (size_t i = 0; i < ast->type->field_count; i++)
		{
			ast->type->field_types[i] = ast->children[0]->type->field_types[i];
		}
		return true;

	// Negatives
	} else if (!strcmp(ast->value.value, "-"))
	{
		// Check the child node and adjust the type of declarations
		if (!check_correctness_helper(ast->children[0], scope, get_real_type, disable_new_vars))
			return false;

		// Find the type of the resulting prefix expression
		type_t* type = scope_lookup_prefix(scope, ast->children[0]->type);
		if (type == NULL)
		{
			printf("Negative sign on invalid operand found at %i:%i\n", ast->children[0]->value.lino, ast->children[0]->value.charpos);
			return false;
		}

		// Set type and return success
		ast->type = type;
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
	create_primatives(scope);

	// Check the correctness of child nodes if top node
	if (ast->value.type == LEX_TYPE_NONE)
	{
		for (int i = 0; i < ast->children_count; i++)
		{
			if (!check_correctness_helper(ast->children[i], scope, false, false))
			{
				while (scope != NULL)
				{
					scope = pop_scope(scope);
				}
				return false;
			}
		}

		pop_scope(scope);
		return true;

	// Check the correctness of the node itself
	} else
	{
		bool result = check_correctness_helper(ast, scope, false, false);
		while (scope != NULL)
		{
			scope = pop_scope(scope);
		}
		return result;
	}
}

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

// generate_type(ast_t*, ir_scope_t*) -> type_t*
// Generates a type from an infix expression.
type_t* generate_type(ast_t* ast, ir_scope_t* scope)
{
	// types:
	// & - intersection type
	// * - product type
	// | - union type
	// >> - function type
	// *type - generator of type
	// [type] - list of type

	// Type symbols
	if (ast->value.type == LEX_TYPE_SYMBOL && ast->children_count == 0)
	{
		type_t* type = scope_lookup_type(scope, ast->value.value);
		if (type == NULL)
		{
			printf("Undeclared type %s found at %i:%i\n", ast->value.value, ast->value.lino, ast->value.charpos);
			return NULL;
		}
		return type;

	// Functions
	} else if (ast->value.type == LEX_TYPE_SYMBOL)
	{
		printf("Functions are unsupported right now\n");
		return NULL;

	// Field declarations (field: type)
	} else if (!strcmp(ast->value.value, ":"))
	{
		// Assert that the left hand side is a symbol
		ast_t* field_name = ast->children[0];
		if (field_name->value.type != LEX_TYPE_SYMBOL)
		{
			printf("Used %s as field name at %i:%i\n", field_name->value.value, field_name->value.lino, field_name->value.charpos);
			return NULL;
		}

		// Get the type of the right hand side
		ast_t* field_type = ast->children[1];
		type_t* subtype = generate_type(field_type, scope);

		// Create a new type
		type_t* type = init_type(IR_TYPES_PRODUCT, NULL, 1);
		type->field_names[0] = strdup(field_name->value.value);
		type->field_types[0] = subtype;
		ast->type = scope_lookup_type(scope, "type");
		return type;

	// Product types
	} else if (!strcmp(ast->value.value, "*"))
	{
		// List of types (reverse order)
		type_t** types = NULL;
		size_t size = 0;
		size_t count = 0;

		do
		{
			// Get type and append it to the list of types
			ast->type = scope_lookup_type(scope, "type");
			type_t* type = generate_type(ast->children[1], scope);
			if (type == NULL) return NULL;
			list_append_element(types, size, count, type_t*, type);

			// Get the next ast and continue if it's also a product
			ast = ast->children[0];
		} while (!strcmp(ast->value.value, "*"));

		// Get the last ast's type and append it to the list of types
		type_t* subtype = generate_type(ast, scope);
		if (subtype == NULL) return NULL;
		list_append_element(types, size, count, type_t*, subtype);

		// Create a new product type and add the subtypes
		type_t* type = init_type(IR_TYPES_PRODUCT, NULL, count);
		for (size_t i = 0; i < count; i++)
		{
			type->field_types[i] = types[count - i - 1];
		}
		return type;

	// Union types
	} else if (!strcmp(ast->value.value, "|"))
	{
		// List of field types and names (reverse order)
		type_t** types = NULL;
		char** names = NULL;
		size_t t_size = 0;
		size_t n_size = 0;
		size_t count = 0;

		do
		{
			// Get type and append it to the list of types
			ast->type = scope_lookup_type(scope, "type");
			type_t* type = generate_type(ast->children[1], scope);
			if (type == NULL) return NULL;
			if (type->field_count == 1 && type->type_type == IR_TYPES_PRODUCT)
			{
				list_append_element(types, t_size, count, type_t*, type->field_types[0]);
				count--;
				list_append_element(names, n_size, count, char*, type->field_names[0]);
			} else
			{
				list_append_element(types, t_size, count, type_t*, type);
				count--;
				list_append_element(names, n_size, count, char*, NULL);
			}

			// Get the next ast and continue if it's also a product
			ast = ast->children[0];
		} while (!strcmp(ast->value.value, "|"));

		// Get the last ast's type and append it to the list of types
		type_t* subtype = generate_type(ast, scope);
		if (subtype == NULL) return NULL;
		if (subtype->field_count == 1 && subtype->type_type == IR_TYPES_PRODUCT)
		{
			list_append_element(types, t_size, count, type_t*, subtype->field_types[0]);
			count--;
			list_append_element(names, n_size, count, char*, subtype->field_names[0]);
		} else
		{
			list_append_element(types, t_size, count, type_t*, subtype);
			count--;
			list_append_element(names, n_size, count, char*, NULL);
		}

		// Create a new product type and add the subtypes
		type_t* type = init_type(IR_TYPES_UNION, NULL, count);
		for (size_t i = 0; i < count; i++)
		{
			type->field_types[i] = types[count - i - 1];
			type->field_names[i] = names[count - i - 1];
		}
		return type;

	// Intersection types
	} else if (!strcmp(ast->value.value, "&"))
	{
		// List of field types and names (reverse order)
		type_t** types = NULL;
		char** names = NULL;
		size_t t_size = 0;
		size_t n_size = 0;
		size_t count = 0;

		do
		{
			// Get type
			ast->type = scope_lookup_type(scope, "type");
			type_t* type = generate_type(ast->children[1], scope);
			if (type == NULL) return NULL;

			// If it's a primitive make an error (cannot intersect primatives)
			if (type->type_type == IR_TYPES_PRIMITIVE)
			{
				printf("Intersection of primitive %s found at %i:%i\n", ast->value.value, ast->value.lino, ast->value.charpos);
				return NULL;
			}

			// Copy over the fields
			for (size_t i = 0; i < type->field_count; i++)
			{
				list_append_element(names, n_size, count, type_t*, type->field_names[i]);
				count--;
				list_append_element(types, t_size, count, type_t*, type->field_types[i]);
			}

			// Get the next ast and continue if it's also a product
			ast = ast->children[0];
		} while (!strcmp(ast->value.value, "&"));

		// Get the last ast's type and append it to the list of types
		type_t* subtype = generate_type(ast, scope);
		if (subtype == NULL) return NULL;

		// If it's a primitive make an error (cannot intersect primatives)
		if (subtype->type_type == IR_TYPES_PRIMITIVE)
		{
			printf("Intersection of primitive %s found at %i:%i\n", ast->value.value, ast->value.lino, ast->value.charpos);
			return NULL;
		}

		// Copy over the fields
		for (size_t i = 0; i < subtype->field_count; i++)
		{
			list_append_element(names, n_size, count, type_t*, subtype->field_names[i]);
			count--;
			list_append_element(types, t_size, count, type_t*, subtype->field_types[i]);
		}

		// Create a new product type and add the subtypes
		type_t* type = init_type(IR_TYPES_INTERSECT, NULL, count);
		for (size_t i = 0; i < count; i++)
		{
			type->field_types[i] = types[count - i - 1];
			type->field_names[i] = names[count - i - 1];
		}
		return type;

	// Function types
	} else if (!strcmp(ast->value.value, ">>"))
	{
		// List of types (reverse order)
		type_t** types = NULL;
		size_t size = 0;
		size_t count = 0;

		do
		{
			// Get type and append it to the list of types
			ast->type = scope_lookup_type(scope, "type");
			type_t* type = generate_type(ast->children[1], scope);
			if (type == NULL) return NULL;
			list_append_element(types, size, count, type_t*, type);

			// Get the next ast and continue if it's also a product
			ast = ast->children[0];
		} while (!strcmp(ast->value.value, ">>"));

		// Get the last ast's type and append it to the list of types
		type_t* subtype = generate_type(ast, scope);
		if (subtype == NULL) return NULL;
		list_append_element(types, size, count, type_t*, subtype);

		// Create a new product type and add the subtypes
		type_t* type = init_type(IR_TYPES_FUNC, NULL, count);
		for (size_t i = 0; i < count; i++)
		{
			type->field_types[i] = types[count - i - 1];
		}
		return type;

	// List types
	} else if (!strcmp(ast->value.value, "[") && ast->children_count == 1)
	{
		// Create list type
		type_t* subtype = generate_type(ast->children[0], scope);
		if (subtype == NULL) return NULL;
		type_t* type = init_type(IR_TYPES_LIST, NULL, 1);
		type->field_types[0] = subtype;
		return type;

	// Error on anything else
	} else
	{
		printf("Invalid type found at %i:%i\n", ast->value.lino, ast->value.charpos);
		return NULL;
	}
}

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

					// Deal with adding new types
					if (types_equal(var_ast->type, scope_lookup_type(scope, "type")))
					{
						if (map_contains(scope->types, var_ast->value.value))
						{
							printf("Mutable type %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
							return false;
						} else
						{
							type_t* type = generate_type(val_ast, scope);
							if (type == NULL) return false;
							type->type_name = strdup(var_ast->value.value);
							print_type(type);
							map_add(scope->types, var_ast->value.value, type);
							return true;
						}
					}

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

			// Construct a type
			type_t* type = generate_type(type_ast, scope);
			if (type == NULL) return false;
			print_type(type);
			var_ast->type = type;

			// Add variable type to the scope
			map_add(scope->var_types, var_ast->value.value, var_ast->type);

			// Get the type of the expression
			ast_t* val_ast = ast->children[1];
			
			// Check for self assignment (var: type = var)
			if (val_ast->value.type == LEX_TYPE_SYMBOL && !strcmp(var_ast->value.value, val_ast->value.value))
			{
				printf("Self assignment of %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
				return false;
			}

			// Deal with adding new types
			if (types_equal(type, scope_lookup_type(scope, "type")))
			{
				// Error on reassigning a previously defined type
				if (map_contains(scope->types, var_ast->value.value))
				{
					printf("Mutable type %s found at %i:%i\n", var_ast->value.value, var_ast->value.lino, var_ast->value.charpos);
					return false;
				} else
				{
					// Add the new type
					type_t* type = generate_type(val_ast, scope);
					if (type == NULL) return false;
					type->type_name = strdup(var_ast->value.value);
					print_type(type);
					map_add(scope->types, var_ast->value.value, type);
					return true;
				}

			// Variable is not a type
			} else
			{
				// Assert the type of the value and variable are the same
				if (!check_correctness_helper(val_ast, scope)) return false;
				if (!types_equal(var_ast->type, val_ast->type))
				{
					printf("Types do not match at %i:%i - %i:%i\n", var_ast->value.lino, var_ast->value.charpos, val_ast->value.lino, val_ast->value.charpos);
					return false;
				}

				// Add variable value to the scope
				map_add(scope->var_vals, var_ast->value.value, val_ast);
				return true;
			}

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

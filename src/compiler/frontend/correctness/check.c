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

// generate_type(ast_t*, ir_scope_t*, ast_t*, type_t*) -> type_t*
// Generates a type from an infix expression.
type_t* generate_type(ast_t* ast, ir_scope_t* scope, ast_t* self, type_t* head)
{
	// types:
	// & - intersection type
	// * - product type
	// | - union type
	// >> - function type
	// *type - generator of type
	// [type] - list of type

	// If the ast is the same as the type name then its equal
	if (asts_equal(ast, self))
		return head;

	// Type symbols
	else if (ast->value.type == LEX_TYPE_SYMBOL)
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
		type_t* type = init_type(IR_TYPES_PRODUCT, NULL, 1);
		if (head == NULL) head = type;
		ast_t* field_type = ast->children[1];
		type_t* subtype = generate_type(field_type, scope, self, head);

		// Create a new type
		type->field_names[0] = strdup(field_name->value.value);
		type->field_types[0] = subtype;
		ast->type = scope_lookup_type(scope, "Type");
		return type;

	// Generator types
	} else if (!strcmp(ast->value.value, "*") && ast->children_count == 1)
	{
		// Create generator type
		type_t* type = init_type(IR_TYPES_GENERATOR, NULL, 1);
		if (head == NULL) head = type;
		type_t* subtype = generate_type(ast->children[0], scope, self, head);
		if (subtype == NULL) return NULL;
		type->field_types[0] = subtype;
		return type;

	// Product types
	} else if (!strcmp(ast->value.value, "*"))
	{
		// List of types (reverse order)
		type_t** types = NULL;
		size_t size = 0;
		size_t count = 0;
		type_t* type = init_type(IR_TYPES_PRODUCT, NULL, 0);
		if (head == NULL) head = type;

		do
		{
			// Get type and append it to the list of types
			ast->type = scope_lookup_type(scope, "Type");
			type_t* type = generate_type(ast->children[1], scope, self, head);
			if (type == NULL) return NULL;
			list_append_element(types, size, count, type_t*, type);

			// Get the next ast and continue if it's also a product
			ast = ast->children[0];
		} while (!strcmp(ast->value.value, "*"));

		// Get the last ast's type and append it to the list of types
		type_t* subtype = generate_type(ast, scope, self, head);
		if (subtype == NULL) return NULL;
		list_append_element(types, size, count, type_t*, subtype);

		// Create a new product type and add the subtypes
		type->field_count = count;
		type->field_names = calloc(count, sizeof(char*));
		type->field_types = calloc(count, sizeof(type_t*));
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
		type_t* type = init_type(IR_TYPES_UNION, NULL, 0);
		if (head == NULL) head = type;

		do
		{
			// Get type and append it to the list of types
			ast->type = scope_lookup_type(scope, "Type");
			type_t* type = generate_type(ast->children[1], scope, self, head);
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
		type_t* subtype = generate_type(ast, scope, self, head);
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
		type->field_count = count;
		type->field_names = calloc(count, sizeof(char*));
		type->field_types = calloc(count, sizeof(type_t*));
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
		type_t* type = init_type(IR_TYPES_PRODUCT, NULL, 0);
		if (head == NULL) head = type;

		do
		{
			// Get type
			ast->type = scope_lookup_type(scope, "Type");
			type_t* type = generate_type(ast->children[1], scope, self, head);
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
		type_t* subtype = generate_type(ast, scope, self, head);
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
		type->field_count = count;
		type->field_names = calloc(count, sizeof(char*));
		type->field_types = calloc(count, sizeof(type_t*));
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
		type_t* type = init_type(IR_TYPES_FUNC, NULL, 0);
		if (head == NULL) head = type;

		do
		{
			// Get type and append it to the list of types
			ast->type = scope_lookup_type(scope, "Type");
			type_t* type = generate_type(ast->children[1], scope, self, head);
			if (type == NULL) return NULL;
			list_append_element(types, size, count, type_t*, type);

			// Get the next ast and continue if it's also a product
			ast = ast->children[0];
		} while (!strcmp(ast->value.value, ">>"));

		// Get the last ast's type and append it to the list of types
		type_t* subtype = generate_type(ast, scope, self, head);
		if (subtype == NULL) return NULL;
		list_append_element(types, size, count, type_t*, subtype);

		// Create a new product type and add the subtypes
		type->field_count = count;
		type->field_names = calloc(count, sizeof(char*));
		type->field_types = calloc(count, sizeof(type_t*));
		for (size_t i = 0; i < count; i++)
		{
			type->field_types[i] = types[count - i - 1];
		}
		return type;

	// List types
	} else if (!strcmp(ast->value.value, "[") && ast->children_count == 1)
	{
		// Create list type
		type_t* type = init_type(IR_TYPES_LIST, NULL, 1);
		if (head == NULL) head = type;
		type_t* subtype = generate_type(ast->children[0], scope, self, head);
		if (subtype == NULL) return NULL;
		type->field_types[0] = subtype;
		return type;

	// Error on anything else
	} else
	{
		printf("Invalid type found at %i:%i\n", ast->value.lino, ast->value.charpos);
		return NULL;
	}
}

// generate_enum(ast_t*, ir_scope_t*) -> type_t*
// Generates an enum type from a given ast.
type_t* generate_enum(ast_t* ast, ir_scope_t* scope, bool save)
{
	// Enum values
	if (ast->value.type == LEX_TYPE_SYMBOL)
	{
		// Create the enum
		type_t* enumy = init_type(IR_TYPES_ENUMERATION, ast->value.value, ast->children_count);
		ast->type = enumy;

		// Add subenums to the enum type
		for (size_t i = 0; i < ast->children_count; i++)
		{
			type_t* subenum = generate_enum(ast->children[i], scope, false);
			if (subenum == NULL) return NULL;
			enumy->field_types[i] = subenum;
		}

		// Save the enum if allowed
		if (save)
		{
			map_add(scope->var_types, ast->value.value, enumy);
			map_add(scope->var_vals, ast->value.value, ast);
		}
		return enumy;

	// Unions
	} else if (!strcmp(ast->value.value, "|"))
	{
		// Save should be true
		if (!save)
		{
			printf("Enum used as parameter of enum found at %i:%i\n", ast->value.lino, ast->value.charpos);
			return NULL;
		}

		// List of field types (reverse order)
		type_t** enums = NULL;
		size_t size = 0;
		size_t count = 0;
		type_t* enumy = init_type(IR_TYPES_UNION, NULL, 0);

		do
		{
			// Get enum and append it to the list of enums
			ast->type = scope_lookup_type(scope, "Enum");
			type_t* enumy = generate_enum(ast->children[1], scope, true);
			if (enumy == NULL) return NULL;
			list_append_element(enums, size, count, type_t*, enumy);

			// Get the next ast and continue if it's also a product
			ast = ast->children[0];
		} while (!strcmp(ast->value.value, "|"));

		// Get the last ast's enum and append it to the list of enums
		type_t* subenum = generate_enum(ast, scope, true);
		if (subenum == NULL) return NULL;
		list_append_element(enums, size, count, type_t*, subenum);

		// Create a new enum type and add the subtypes
		enumy->field_count = count;
		enumy->field_names = calloc(count, sizeof(char*));
		enumy->field_types = calloc(count, sizeof(type_t*));
		for (size_t i = 0; i < count; i++)
		{
			enumy->field_types[i] = enums[count - i - 1];
		}
		return enumy;

	// Error on anything else
	} else
	{
		printf("Invalid type found at %i:%i\n", ast->value.lino, ast->value.charpos);
		return NULL;
	}
}

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
		// Check the applicator
		if (!check_correctness_helper(ast->children[0], scope, get_real_type, disable_new_vars))
			return false;
		printf("Function application is unsupported at the moment");

	// Assign nodes with undeclared variables
	} else if (ast->value.type == LEX_TYPE_ASSIGN)
	{
		// var = expr (cannot be recursive)
		if (ast->children[0]->value.type == LEX_TYPE_SYMBOL)
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
						print_type(type);
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
				print_type(type);
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
			print_type(type);
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
					print_type(type);
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
					type_t* type = generate_enum(val_ast, scope, true);
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
				if (!check_correctness_helper(val_ast, scope, get_real_type, disable_new_vars)) return false;
				if (!type_subtype(var_ast->type, val_ast->type, true))
				{
					printf("Types do not match at %i:%i - %i:%i\n", var_ast->value.lino, var_ast->value.charpos, val_ast->value.lino, val_ast->value.charpos);
					return false;
				}

				// Add variable value to the scope
				print_type(val_ast->type);
				map_add(scope->var_vals, var_ast->value.value, val_ast);
				return true;
			}

		// x..xs = expr (cannot be recursive)
		} else if (ast->children[0]->value.type == LEX_TYPE_RANGE)
		{
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

		// TODO: range and function assignment
		} else return false;

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

		// TODO: getting elements from lists
		} else return false;

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
				return false;
		}
		return true;

	// Check the correctness of the node itself
	} else return check_correctness_helper(ast, scope, false, false);
}

// 
// correctness
// type_generators.c: Provides functions for generating types.
// 
// Created by jenra.
// Created on September 5 2020.
// 

#include <stdio.h>
#include <string.h>

#include "../../../utils/list.h"
#include "type_generators.h"

// generate_type(ast_t*, ir_scope_t*, ast_t*, type_t*) -> type_t*
// Generates a type from an infix expression.
// TODO: parametric types (both recursive and nonrecursive)
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

		// Create a new union type and add the subtypes
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

		// Create a new intersection type and add the subtypes
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
		// Fill final return type
		type_t* type = init_type(IR_TYPES_FUNC, NULL, 2);
		if (head == NULL) head = type;
		type->field_types[1] = generate_type(ast->children[1], scope, self, head);
		if (type->field_types[1] == NULL)
			return NULL;

		while (true)
		{
			// Get next ast node
			ast = ast->children[0];
			if (strcmp(ast->value.value, ">>"))
				break;

			// Fill argument
			type->field_types[0] = generate_type(ast->children[1], scope, self, head);
			if (type->field_types[0] == NULL)
				return NULL;

			// Generate the function returning the current function
			type_t* t = init_type(IR_TYPES_FUNC, NULL, 2);
			t->field_types[1] = type;
			type = t;
		}

		// Fill argument
		type->field_types[0] = generate_type(ast, scope, self, head);
		if (type->field_types[0] == NULL)
			return NULL;
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

// generate_enum(ast_t*, ir_scope_t*, type_t*) -> type_t*
// Generates an enum type from a given ast.
type_t* generate_enum(ast_t* ast, ir_scope_t* scope, type_t* head)
{
	// Enum values
	if (ast->value.type == LEX_TYPE_SYMBOL && ast->children_count == 0)
	{
		// Create the enum
		type_t* enumy = init_type(IR_TYPES_ENUMERATION, ast->value.value, 0); // ast->children_count);
		ast->type = enumy;

		map_add(scope->var_types, ast->value.value, head);
		map_add(scope->var_vals, ast->value.value, ast);

		return enumy;

	// Unions
	} else if (!strcmp(ast->value.value, "|"))
	{
		// List of field types (reverse order)
		type_t** enums = NULL;
		size_t size = 0;
		size_t count = 0;
		type_t* enumy = init_type(IR_TYPES_UNION, NULL, 0);
		if (head == NULL) head = enumy;

		do
		{
			// Get enum and append it to the list of enums
			ast->type = scope_lookup_type(scope, "Enum");
			type_t* enumy = generate_enum(ast->children[1], scope, head);
			if (enumy == NULL) return NULL;
			list_append_element(enums, size, count, type_t*, enumy);

			// Get the next ast and continue if it's also a product
			ast = ast->children[0];
		} while (!strcmp(ast->value.value, "|"));

		// Get the last ast's enum and append it to the list of enums
		type_t* subenum = generate_enum(ast, scope, head);
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
		printf("Invalid enum found at %i:%i\n", ast->value.lino, ast->value.charpos);
		return NULL;
	}
}

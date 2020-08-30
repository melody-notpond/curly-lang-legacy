// 
// ir-codegen
// types.c: Implements different functions for comparing, combining, and representing types in curly.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#include <stdio.h>
#include <string.h>

#include "types.h"

type_t* type_linked_list_head = NULL;

// create_primatives(void) -> void
// Creates the builtin primative types.
void create_primatives()
{
	// Primatives must be the first types created
	if (type_linked_list_head != NULL)
		return;

	// Create primatives
	init_type(IR_TYPES_PRIMITIVE, "int", 0);
	init_type(IR_TYPES_PRIMITIVE, "float", 0);
	init_type(IR_TYPES_PRIMITIVE, "string", 0);
	init_type(IR_TYPES_PRIMITIVE, "bool", 0);
	init_type(IR_TYPES_PRIMITIVE, "dict", 0);
	init_type(IR_TYPES_PRIMITIVE, "obj", 0);
	init_type(IR_TYPES_PRIMITIVE, "type", 0);
}

// init_type(ir_type_types_t, char*, size_t, type_t) -> type_t*
// Initialises a new type.
type_t* init_type(ir_type_types_t type_type, char* name, size_t field_count)
{
	type_t* type = malloc(sizeof(type_t));
	type->type_type = type_type;
	type->type_name = name != NULL ? strdup(name) : NULL;
	type->field_types = calloc(field_count, sizeof(type_t*));
	type->field_names = calloc(field_count, sizeof(char*));
	type->field_count = field_count;

	// Add to linked list
	type->next = type_linked_list_head;
	type_linked_list_head = type;
	return type;
}

// type_get_return_type(type_t*) -> type_t*
// Get the return type of a function type.
type_t* type_get_return_type(type_t* type)
{
	while (type->type_type == IR_TYPES_FUNC && type->field_count > 0)
	{
		type = type->field_types[type->field_count - 1];
	}
	return type;
}

// types_equal(type_t*, type_t*) -> bool
// Returns whether the two types are equal or not.
bool types_equal(type_t* t1, type_t* t2)
{
	if (t1 == NULL || t2 == NULL)
		return t1 == t2;

	// Types must be the same type of type and have the same number of fields
	if (t1->type_type != t2->type_type || t1->field_count != t2->field_count)
		return false;

	// Every time is equal to itself
	else if (t1 == t2)
		return true;

	// Check the contents of the type
	switch (t1->type_type)
	{
		case IR_TYPES_PRIMITIVE:
			// Primatives are equal if they have the same name
			return !strcmp(t1->type_name, t2->type_name);
		case IR_TYPES_PRODUCT:
		case IR_TYPES_INTERSECT:
		case IR_TYPES_UNION:
		case IR_TYPES_LIST:
		case IR_TYPES_FUNC:
			// Compound types are equal if their field types are the same
			for (int i = 0; i < t1->field_count; i++)
			{
				bool equal = types_equal(t1->field_types[i], t2->field_types[i]);
				if (!equal) return false;
			}
			return true;
		default:
			return false;
	}
}

// print_type_helper(type_t*, int) -> void
// Helps to print out a type's internal structure.
void print_type_helper(type_t* type, char* name, int level)
{
	// Print indentation
	for (int i = 0; i < level; i++)
	{
		putc('\t', stdout);
	}
	if (level > 0) printf("| ");

	// Stop after 3 levels (recursive data structures aren't nice to print out...)
	if (level > 3)
	{
		puts("...");
		return;
	}

	// Print out the field name if applicable
	if (name != NULL)
		printf("%s: ", name);

	// Print out the type of the type
	switch (type->type_type)
	{
		case IR_TYPES_PRIMITIVE:
			printf("prim");
			break;
		case IR_TYPES_PRODUCT:
			printf("prod");
			break;
		case IR_TYPES_INTERSECT:
			printf("inter");
			break;
		case IR_TYPES_UNION:
			printf("union");
			break;
		case IR_TYPES_LIST:
			printf("list");
			break;
		case IR_TYPES_GENERATOR:
			printf("gen");
			break;
		case IR_TYPES_FUNC:
			printf("func");
			break;
		default:
			printf("type");
			break;
	}

	// Print out the type name if applicable
	if (type->type_name != NULL)
		printf(" %s", type->type_name);
	puts("");

	// Print out children
	for (int i = 0; i < type->field_count; i++)
	{
		print_type_helper(type->field_types[i], type->field_names[i], level + 1);
	}
}

// print_type(type_t*) -> void
// Prints out a type.
void print_type(type_t* type) { print_type_helper(type, NULL, 0); }

// clean_types(void) -> void
// Frees every type created.
void clean_types()
{
	// Iterate over every
	while (type_linked_list_head != NULL)
	{
		// Free fields
		free(type_linked_list_head->type_name);
		free(type_linked_list_head->field_types);
		free(type_linked_list_head->field_names);

		// Free head pointer
		type_t* tail = type_linked_list_head->next;
		free(type_linked_list_head);
		type_linked_list_head = tail;
	}
}

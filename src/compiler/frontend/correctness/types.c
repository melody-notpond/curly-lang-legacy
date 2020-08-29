// 
// ir-codegen
// types.c: Implements different functions for comparing, combining, and representing types in curly.
// 
// Created by jenra.
// Created on August 29 2020.
// 

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
	init_type(IR_TYPES_PRIMITIVE, "list", 0);
	init_type(IR_TYPES_PRIMITIVE, "dict", 0);
	init_type(IR_TYPES_PRIMITIVE, "type", 0);
	init_type(IR_TYPES_PRIMITIVE, "func", 0);
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

// types_equal(type_t*, type_t*) -> bool
// Returns whether the two types are equal or not.
bool types_equal(type_t* t1, type_t* t2)
{
	// Types must be the same type of type and have the same number of fields
	if (t1->type_type != t2->type_type || t1->field_count != t2->field_count)
		return false;

	// Every time is equal to itself
	else if (t1 == t2)
		return true;

	// Check the contents of the type
	switch (t1->type_type)
	{
		IR_TYPES_PRIMITIVE:
			// Primatives are equal if they have the same name
			return !strcmp(t1->type_name, t2->type_name);
		IR_TYPES_PRODUCT:
		IR_TYPES_INTERSECT:
		IR_TYPES_UNION:
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

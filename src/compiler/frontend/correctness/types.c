// 
// ir-codegen
// types.c: Implements different functions for comparing, combining, and representing types in curly.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#include <stdio.h>
#include <string.h>

#include "scope.h"
#include "types.h"

static type_t* type_linked_list_head = NULL;

// create_primatives(ir_scope_t*) -> void
// Creates the builtin primative types.
void create_primatives(ir_scope_t* scope)
{
	// Primatives must be the first types created
	if (type_linked_list_head != NULL)
		return;

	// Create primatives
	type_t* _int = init_type(IR_TYPES_PRIMITIVE, "Int", 0);
	type_t* _float = init_type(IR_TYPES_PRIMITIVE, "Float", 0);
	init_type(IR_TYPES_PRIMITIVE, "String", 0);
	init_type(IR_TYPES_PRIMITIVE, "Bool", 0);
	//init_type(IR_TYPES_PRIMITIVE, "Dict", 0);
	init_type(IR_TYPES_PRIMITIVE, "Enum", 0);

	// Add to scope
	type_t* head = type_linked_list_head;
	while (head != NULL)
	{
		map_add(scope->types, head->type_name, head);
		head = head->next;
	}

	// Define default arithmetic infix operations
	add_infix_op(scope, IR_BINOPS_MUL, _int, _int, _int);
	add_infix_op(scope, IR_BINOPS_MUL, _float, _int, _float);
	add_infix_op(scope, IR_BINOPS_MUL, _int, _float, _float);
	add_infix_op(scope, IR_BINOPS_MUL, _float, _float, _float);
	add_infix_op(scope, IR_BINOPS_DIV, _int, _int, _int);
	add_infix_op(scope, IR_BINOPS_DIV, _float, _int, _float);
	add_infix_op(scope, IR_BINOPS_DIV, _int, _float, _float);
	add_infix_op(scope, IR_BINOPS_DIV, _float, _float, _float);
	add_infix_op(scope, IR_BINOPS_MOD, _int, _int, _int);
	add_infix_op(scope, IR_BINOPS_ADD, _int, _int, _int);
	add_infix_op(scope, IR_BINOPS_ADD, _float, _int, _float);
	add_infix_op(scope, IR_BINOPS_ADD, _int, _float, _float);
	add_infix_op(scope, IR_BINOPS_ADD, _float, _float, _float);
	add_infix_op(scope, IR_BINOPS_SUB, _int, _int, _int);
	add_infix_op(scope, IR_BINOPS_SUB, _float, _int, _float);
	add_infix_op(scope, IR_BINOPS_SUB, _int, _float, _float);
	add_infix_op(scope, IR_BINOPS_SUB, _float, _float, _float);
	add_infix_op(scope, IR_BINOPS_BSL, _int, _int, _int);
	add_infix_op(scope, IR_BINOPS_BSR, _int, _int, _int);
	add_infix_op(scope, IR_BINOPS_BITAND, _int, _int, _int);
	add_infix_op(scope, IR_BINOPS_BITOR, _int, _int, _int);
	add_infix_op(scope, IR_BINOPS_BITXOR, _int, _int, _int);

	// Define default prefix operations
	add_prefix_op(scope, _int, _int);
	add_prefix_op(scope, _float, _float);
}

// init_type(ir_type_types_t, char*, size_t, type_t) -> type_t*
// Initialises a new type.
type_t* init_type(ir_type_types_t type_type, char* name, size_t field_count)
{
	type_t* type = malloc(sizeof(type_t));
	type->printing = false;
	type->name_carry = NULL;
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

// type_subtype(type_t*, type_t*, bool) -> bool
// Returns true if the second type is a valid type under the first type.
bool type_subtype(type_t* super, type_t* sub, bool override_fields)
{
	if (super == NULL || sub == NULL)
		return super == sub;

	// Empty lists are valid subtypes of populated lists
	if (super->type_type == IR_TYPES_LIST && sub->type_type == IR_TYPES_LIST && sub->field_count == 0)
		return true;

	// Nonunion types must be the same type of type and have the same number of fields
	else if (super->type_type != IR_TYPES_UNION && (super->type_type != sub->type_type || super->field_count != sub->field_count))
		return false;

	// Every type is equal to itself
	else if (super == sub)
		return true;

	// Check the contents of the type
	switch (super->type_type)
	{
		case IR_TYPES_PRIMITIVE:
		case IR_TYPES_ENUMERATION:
			// Primatives and enums check if they have the same name and number of subtypes
			return !strcmp(super->type_name, sub->type_name);
		case IR_TYPES_UNION:
			// Union types check its subtypes against the passed subtype
			for (size_t i = 0; i < super->field_count; i++)
			{
				bool equal = type_subtype(super->field_types[i], sub, override_fields);
				if (equal)
				{
					if (override_fields)
						sub->name_carry = super->field_names[i];
					return true;
				}
			}
			return false;
		case IR_TYPES_PRODUCT:
		case IR_TYPES_LIST:
		case IR_TYPES_GENERATOR:
		case IR_TYPES_FUNC:
		case IR_TYPES_CURRY:
			// Compound types are equal if their field types are the same
			for (size_t i = 0; i < super->field_count; i++)
			{
				bool equal = (super->field_names[i] != NULL && sub->field_names[i] != NULL ? !strcmp(super->field_names[i], sub->field_names[i]) : true) && type_subtype(super->field_types[i], sub->field_types[i], override_fields);
				if (!equal) return false;

				// Force null labels to match
				if (override_fields)
				{
					if (sub->field_names[i] == NULL)
					{
						if (sub->field_types[i]->name_carry != NULL)
						{
							sub->field_names[i] = strdup(sub->field_types[i]->name_carry);
							sub->field_types[i]->name_carry = NULL;
						} else if (super->field_names[i] != NULL)
							sub->field_names[i] = strdup(super->field_names[i]);
					}
				}
			}

			return true;
		default:
			return false;
	}
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

	// Every type is equal to itself
	else if (t1 == t2)
		return true;

	// Check the contents of the type
	switch (t1->type_type)
	{
		case IR_TYPES_PRIMITIVE:
		case IR_TYPES_ENUMERATION:
			// Primatives and enums are equal if they have the same name
			return !strcmp(t1->type_name, t2->type_name);
		case IR_TYPES_PRODUCT:
		case IR_TYPES_UNION:
		case IR_TYPES_LIST:
		case IR_TYPES_GENERATOR:
		case IR_TYPES_FUNC:
		case IR_TYPES_CURRY:
			// Compound types are equal if their field types are the same
			for (size_t i = 0; i < t1->field_count; i++)
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

	if (type == NULL)
	{
		puts("null");
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
		case IR_TYPES_ENUMERATION:
			printf("enum");
			break;
		case IR_TYPES_PRODUCT:
			printf("prod");
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

	// If the type is currently being printed out, stop printing it a second time
	if (type->printing)
	{
		// Print indentation
		for (int i = 0; i < level + 1; i++)
		{
			putc('\t', stdout);
		}
		puts("| ...");
		return;
	} else type->printing = true;

	// Print out children
	for (int i = 0; i < type->field_count; i++)
	{
		print_type_helper(type->field_types[i], type->field_names[i], level + 1);
	}

	// Set printing to false
	type->printing = false;
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

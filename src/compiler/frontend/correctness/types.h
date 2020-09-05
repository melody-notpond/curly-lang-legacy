//
// ir-codegen
// types.h: Header file for types.c.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#ifndef types_h
#define types_h

#include <stdlib.h>
#include <stdbool.h>

#include "../../../utils/hashmap.h"

// Represents the types of types.
typedef enum
{
	IR_TYPES_PRIMITIVE,
	IR_TYPES_ENUMERATION,
	IR_TYPES_PRODUCT,
	IR_TYPES_UNION,
	IR_TYPES_LIST,
	IR_TYPES_GENERATOR,
	IR_TYPES_FUNC,
	IR_TYPES_CURRY
} ir_type_types_t;

// Represents a curly type in the intermediate representation.
typedef struct s_type
{
	// If the type is currently being printed
	bool printing;

	// Carries the name from this type node to its parent.
	char* name_carry;

	// The type of type
	ir_type_types_t type_type;

	// The name of the type
	char* type_name;

	// Array of field names (names can be null)
	char** field_names;
	struct s_type** field_types;
	size_t field_count;

	// Linked list of types
	struct s_type* next;
} type_t;

typedef struct s_ir_scope ir_scope_t;

// create_primatives(ir_scope_t*) -> void
// Creates the builtin primative types.
void create_primatives(ir_scope_t* scope);

// init_type(ir_type_types_t, char*, size_t) -> type_t*
// Initialises a new type.
type_t* init_type(ir_type_types_t type_type, char* name, size_t field_count);

// type_subtype(type_t*, type_t*, bool) -> bool
// Returns true if the second type is a valid type under the first type.
bool type_subtype(type_t* super, type_t* sub, bool override_fields);

// types_equal(type_t*, type_t*) -> bool
// Returns whether the two types are equal or not.
bool types_equal(type_t* t1, type_t* t2);

// print_type(type_t*) -> void
// Prints out a type.
void print_type(type_t* type);

// clean_types(void) -> void
// Frees every type created.
void clean_types();

#endif /* types_h */

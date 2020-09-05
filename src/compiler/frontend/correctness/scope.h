//
// correctness
// scope.h: Header file for scope.c.
//
// Created by jenra.
// Created on August 29 2020.
//

#ifndef SCOPE_H
#define SCOPE_H

#include "../../../utils/hashmap.h"
#include "../parse/ast.h"
#include "types.h"

#define INFIX_OP_COUNT 11

// Represents an infix operator's type
typedef struct s_ir_infix_type
{
	// The type of the infix value
	type_t* type;

	// The next infix type in the linked list.
	struct s_ir_infix_type* next;
} ir_infix_type_t;

// Represents a scope.
typedef struct s_ir_scope
{
	// A hashmap of all variable names mapped to their types in the current scope.
	hashmap_t* var_types;

	// A hashmap of all variable names mapped to their values in the current scope.
	hashmap_t* var_vals;

	// A hashmap of all the types mapped by their names in the current scope.
	hashmap_t* types;

	// The infix operations defined for the current scope.
	ir_infix_type_t* infix_ops[INFIX_OP_COUNT];

	// The parent scope.
	struct s_ir_scope* parent;
} ir_scope_t;

// push_scope(ir_scope_t*) -> ir_scope_t*
// Initialises a scope and pushes it onto the stack of scopes.
ir_scope_t* push_scope(ir_scope_t* parent);

// add_prefix_op(ir_scope_t*, type_t*, type_t*) -> void
// Adds a new prefix operator to the scope.
void add_prefix_op(ir_scope_t* scope, type_t* operand, type_t* out);

// add_infix_op(ir_scope_t*, char*, type_t*, type_t*, type_t*) -> void
// Adds an infix operation to the scope.
void add_infix_op(ir_scope_t* scope, char* op, type_t* left, type_t* right, type_t* out);

// scope_lookup_prefix(ir_scope_t*, type_t*) -> type_t*
// Looks up a prefix operator based on the argument type and returns the result type.
type_t* scope_lookup_prefix(ir_scope_t* scope, type_t* operand);

// scope_lookup_infix(ir_scope_t*, char*, type_t*, type_t*) -> type_t*
// Looks up an infix operator based on the argument types and returns the result type.
type_t* scope_lookup_infix(ir_scope_t* scope, char* op, type_t* left, type_t* right);

// scope_lookup_var_type(ir_scope_t*, char*) -> type_t*
// Looks up the type of a variable in the scope.
type_t* scope_lookup_var_type(ir_scope_t* scope, char* name);

// scope_lookup_var_val(ir_scope_t*, char*) -> ast_t*
// Looks up the value of a variable in the scope.
ast_t* scope_lookup_var_val(ir_scope_t* scope, char* name);

// scope_lookup_type(ir_scope_t*, char*) -> type_t*
// Looks up a type in the scope.
type_t* scope_lookup_type(ir_scope_t* scope, char* name);

// pop_scope(ir_scope_t* scope)
// Deletes a scope and returns its parent.
ir_scope_t* pop_scope(ir_scope_t* scope);

#endif /* SCOPE_H */

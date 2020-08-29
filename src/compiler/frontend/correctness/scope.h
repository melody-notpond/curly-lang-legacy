//
// correctness
// scope.h: Header file for scope.c.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#ifndef scope_h
#define scope_h

#include "../../../utils/hashmap.h"
#include "types.h"

// Represents a scope.
typedef struct s_ir_scope
{
	// A hashmap of all variable names mapped to their types in the current scope.
	hashmap_t* variables;

	// A hashmap of all the types mapped by their names in the current scope.
	hashmap_t* types;

	// The parent scope.
	struct s_ir_scope* parent;
} ir_scope_t;

// push_scope(ir_scope_t*) -> ir_scope_t*
// Initialises a scope and pushes it onto the stack of scopes.
ir_scope_t* push_scope(ir_scope_t* parent);

// pop_scope(ir_scope_t* scope)
// Deletes a scope and returns its parent.
ir_scope_t* pop_scope(ir_scope_t* scope)

#endif /* scope_h */

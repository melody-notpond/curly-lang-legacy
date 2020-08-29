// 
// correctness
// scope.c: Implements a scope system.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#include "scope.h"

// push_scope(ir_scope_t*) -> ir_scope_t*
// Initialises a scope and pushes it onto the stack of scopes.
ir_scope_t* push_scope(ir_scope_t* parent)
{
	ir_scope_t* scope = malloc(sizeof(ir_scope_t));
	scope->variables = init_hashmap();
	scope->types = init_hashmap();
	scope->parent = parent;
	return scope;
}

// pop_scope(ir_scope_t* scope)
// Deletes a scope and returns its parent.
ir_scope_t* pop_scope(ir_scope_t* scope)
{
	del_hashmap(scope->variables);
	del_hashmap(scope->types);
	ir_scope_t* parent = scope->parent;
	free(scope);
	return parent;
}

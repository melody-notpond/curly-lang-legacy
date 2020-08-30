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
	scope->var_types = init_hashmap();
	scope->var_vals = init_hashmap();
	scope->types = init_hashmap();
	scope->parent = parent;
	return scope;
}

// scope_lookup_var_type(ir_scope_t*, char*) -> type_t*
// Looks up the type of a variable in the scope.
type_t* scope_lookup_var_type(ir_scope_t* scope, char* name)
{
	// Iterate over every scope
	while (scope != NULL)
	{
		// Get the type in the current scope
		type_t* type = map_get(scope->var_types, name);

		// Return the type if found
		if (type != NULL)
			return type;

		// Get parent scope
		scope = scope->parent;
	}

	// No type was found
	return NULL;
}

// scope_lookup_var_val(ir_scope_t*, char*) -> ast_t*
// Looks up the value of a variable in the scope.
ast_t* scope_lookup_var_val(ir_scope_t* scope, char* name)
{
	// Iterate over every scope
	while (scope != NULL)
	{
		// Get the ast in the current scope
		ast_t* ast = map_get(scope->var_types, name);

		// Return the ast if found
		if (ast != NULL)
			return ast;

		// Get parent scope
		scope = scope->parent;
	}

	// No ast was found
	return NULL;
}

// scope_lookup_type(ir_scope_t*, char*) -> type_t*
// Looks up a type in the scope.
type_t* scope_lookup_type(ir_scope_t* scope, char* name)
{
	// Iterate over every scope
	while (scope != NULL)
	{
		// Get the type in the current scope
		type_t* type = map_get(scope->types, name);

		// Return the type if found
		if (type != NULL)
			return type;

		// Get parent scope
		scope = scope->parent;
	}

	// No type was found
	return NULL;
}

// pop_scope(ir_scope_t* scope)
// Deletes a scope and returns its parent.
ir_scope_t* pop_scope(ir_scope_t* scope)
{
	del_hashmap(scope->var_types);
	del_hashmap(scope->var_vals);
	del_hashmap(scope->types);
	ir_scope_t* parent = scope->parent;
	free(scope);
	return parent;
}

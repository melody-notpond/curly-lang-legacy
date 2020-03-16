//
// Curly
// scopes.c: Implements various tools to help backends check that scopes are used correctly.
//
// jenra
// March 15 2020
//

#include <stdlib.h>

#include "scopes.h"

// init_scopes(scopes_t*) -> void
// Initialises a stack of scopes.
void init_scopes(scopes_t* scopes)
{
	scopes->global.call = false;
	scopes->global.names = NULL;
	scopes->global.types = NULL;
	scopes->global.size = 0;
	scopes->global.count = 0;
	scopes->global.last = NULL;
	scopes->local = &scopes->global;
}

// push_scope(scopes_t*, bool) -> void
// Pushes a new scope into a stack of scopes.
void push_scope(scopes_t* scopes, bool from_call)
{
	struct s_scope* local = malloc(sizeof(struct s_scope));
	local->call = from_call;
	local->names = NULL;
	local->types = NULL;
	local->size = 0;
	local->count = 0;
	local->last = scopes->local;
	scopes->local = local;
}

// pop_scope(scopes_t*) -> bool
// Pops a scope from a stack of scopes.
// Returns true if a local scope was popped.
bool pop_scope(scopes_t* scopes)
{
	if (scopes->local == &scopes->global)
		return false;

	struct s_scope* local = scopes->local;
	scopes->local = local->last;

	for (int i = 0; i < local->count; i++)
	{
		free(local->names[i]);
	}

	free(local->names);
	free(local->types);
	free(local);

	return true;
}

// clean_scopes(scopes_t*) -> void
// Cleans a stack of scopes.
void clean_scopes(scopes_t* scopes)
{
	while (pop_scope(scopes));

	for (int i = 0; i < scopes->global.call; i++)
	{
		free(scopes->global.names[i]);
	}
	free(scopes->global.names);
	free(scopes->global.types);

	scopes->global.names = NULL;
	scopes->global.types = NULL;
	scopes->global.size = 0;
	scopes->global.count = 0;
}

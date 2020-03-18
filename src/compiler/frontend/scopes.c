//
// Curly
// scopes.c: Implements various tools to help backends check that scopes are used correctly.
//
// jenra
// March 16 2020
//

#include <stdlib.h>
#include <strings.h>

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

// add_variable(scopes_t*, char*, curly_type_t) -> bool
// Adds a variable to the current scope. Returns true if the variable did not exist prior.
bool add_variable(scopes_t* scopes, char* name, curly_type_t type)
{
	struct s_scope* local = scopes->local;

	// Linear search through the scope's variables
	for (int i = 0; i < local->count; i++)
	{
		if (!strcmp(name, local->names[i]))
			return false;
	}

	// Resize the lists if necessary
	if (local->size == 0)
	{
		local->names = calloc((local->size = 8), sizeof(char*));
		local->types = calloc((local->size), sizeof(curly_type_t));
	} else if (local->count >= local->size)
	{
		local->names = realloc(local->names, (local->size <<= 1) * sizeof(char*));
		local->types = realloc(local->names, (local->size) * sizeof(curly_type_t*));
	}

	// Add the variable to the scope
	local->names[local->count  ] = strdup(name);
	local->types[local->count++] = type;
	return true;
}

// search_local(scopes_t*, char*) -> struct s_local_search_res
// Searches for a variable name in the locals. Returns -1 if not found.
struct s_local_search_res search_local(scopes_t* scopes, char* name)
{
	struct s_scope* local = scopes->local;

	int depth = 0;
	while (local != &scopes->global)
	{
		// Linear search through the locals
		for (int i = 0; i < local->count; i++)
		{
			if (!strcmp(name, local->names[i]))
				return (struct s_local_search_res) {depth, i, local->types[i]};
		}

		// Local does not exist
		if (local->call || local->last == NULL)
		return (struct s_local_search_res) {-1, -1, SCOPE_CURLY_TYPE_DNE};

		// Keep searching
		else local = local->last;
		depth++;
	}

	// At this point, we're in the global scope
	return (struct s_local_search_res) {-1, -1, SCOPE_CURLY_TYPE_DNE};
}

// search_global(scopes_t*, char*) -> int
// Searches for a variable name in the globals. Returns -1 if not found.
int search_global(scopes_t* scopes, char* name)
{
	struct s_scope global = scopes->global;

	// Linear search through the globals
	for (int i = 0; i < global.count; i++)
	{
		if (!strcmp(name, global.names[i]))
			return i;
	}

	// Global does not exist
	return -1;
}

// pop_scope(scopes_t*) -> bool
// Pops a scope from a stack of scopes. Returns true if a local scope was popped.
bool pop_scope(scopes_t* scopes)
{
	// Don't pop the global scope.
	if (scopes->local == &scopes->global)
		return false;

	// The scope below the local scope is the new local scope
	struct s_scope* local = scopes->local;
	scopes->local = local->last;

	// Free names
	for (int i = 0; i < local->count; i++)
	{
		free(local->names[i]);
	}

	// Other frees
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

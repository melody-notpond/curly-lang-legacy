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

// clean_scopes(scopes_t*) -> void
// Cleans a stack of scopes.
void clean_scopes(scopes_t* scopes)
{
	struct s_scope* local = scopes->local;

	while (local != NULL)
	{
		for (int i = 0; i < local->count; i++)
		{
			free(local->names[i]);
		}
		free(local->names);
		free(local->types);

		struct s_scope* last = local->last;
		if (last != NULL)
			free(local);
		local = last;
	}

	scopes->global.names = NULL;
	scopes->global.types = NULL;
	scopes->global.size = 0;
	scopes->global.count = 0;
}

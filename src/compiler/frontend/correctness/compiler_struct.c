//
// Curly
// compiler_struct.c: Implements the compiler_t structure, which holds compilation state.
//
// jenra
// March 16 2020
//

#include "compiler_struct.h"

// init_compiler_state(compiler_t*) -> void
// Initialises a compiler state.
void init_compiler_state(compiler_t* state)
{
	state->ir = init_ir();
	init_scopes(&state->scope);
	state->status = COMPILE_STATUS_SUCC;
	state->type_cause = SCOPE_CURLY_TYPE_DNE;
	state->cause = NULL;
}

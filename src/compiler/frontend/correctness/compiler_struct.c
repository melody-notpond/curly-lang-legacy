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
	state->type_cause = CURLY_TYPE_DNE;
	state->cause = NULL;
}

// print_compile_error(compiler_t*, char*, char*) -> void
// Prints out a message for the compiler.
void print_compile_error(compiler_t* state, char* file, char* string)
{
	// Don't print out errors for successful conversions
	if (state->status == COMPILE_STATUS_SUCC)
		return;

	// Print out location
	printf("%s (%i:%i): ", file, state->cause->line, state->cause->char_pos);

	// Print out the error specific message
	switch (state->status)
	{
		case COMPILE_STATUS_INFIX_NOT_NUMBER:
			printf("Error: expected a number, got type %s instead\n", curly_type_as_string(state->type_cause));
			break;
		case COMPILE_STATUS_MOD_WITH_FLOATS:
			printf("Error: can't use modulo on a float\n");
			break;
		default:
			printf("Error: An unknown error occured\n");
			break;
	}

	// Search for the line
	char* line = string;
	int line_count = state->cause->line - 1;
	while (line_count)
	{
		line++;
		if (*line == '\n')
		{
			line_count--;
			line++;
		}
	}

	// Print out the line
	for (char* c = line; *c && *c != '\n'; c++) printf("%c", *c);
	puts("");

	// Print out the pointer thing
	for (int i = 0; i < state->cause->char_pos; i++, line++)
	{
		printf("%c", (*line == '\t' ? '\t' : ' '));
	}
	puts("^");
}

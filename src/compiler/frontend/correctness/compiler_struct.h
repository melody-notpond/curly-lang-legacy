//
// Curly
// compiler_struct.h: Implements the compiler_t structure, which holds compilation state.
//
// jenra
// March 16 2020
//

#ifndef compiler_struct_h
#define compiler_struct_h

#include "../ir-codegen/ir.h"
#include "combinator.h"
#include "scopes.h"

typedef enum
{
	COMPILE_STATUS_SUCC,
	COMPILE_STATUS_INFIX_NOT_NUMBER,
	COMPILE_STATUS_MOD_WITH_FLOATS
} compile_status_t;

// Represents the compiler state.
typedef struct
{
	// The ir being generated.
	curly_ir_t ir;

	// The current scope.
	scopes_t scope;

	// The status of the compiler.
	compile_status_t status;

	// The type that caused the error, if any.
	curly_type_t type_cause;

	// The cause of the error, if any.
	ast_t* cause;
} compiler_t;

// init_compiler_state(compiler_t*) -> void
// Initialises a compiler state.
void init_compiler_state(compiler_t* state);

// print_compile_error(compiler_t*, char*, char*) -> void
// Prints out a message for the compiler.
void print_compile_error(compiler_t* state, char* file, char* string);

#endif /* compiler_struct_h */

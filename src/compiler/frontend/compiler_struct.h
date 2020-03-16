//
// Curly
// compiler_struct.h: Header file for compiler_struct.c.
//
// jenra
// March 16 2020
//

#ifndef compiler_struct_h
#define compiler_struct_h

#include "combinator.h"
#include "scopes.h"

// Represents the compiler state.
typedef struct
{
	// The current scope.
	scopes_t scope;

	// The status of the compiler.
	int status;

	// The type that caused the error, if any.
	curly_type_t type_cause;

	// The cause of the error, if any.
	ast_t* cause;
} compiler_t;

#endif /* compiler_struct_h */

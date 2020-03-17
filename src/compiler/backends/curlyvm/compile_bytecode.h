//
// Curly
// compile_bytecode.h: Header file for compile_bytecode.c.
//
// jenra
// March 15 2020
//

#ifndef compile_bytecode_h
#define compile_bytecode_h

#include "combinator.h"
#include "../../frontend/compiler_struct.h"
#include "../../../vm/bytecode.h"

// Represents the compiler state.
typedef struct
{
	// The chunk of bytecode being compiled into.
	chunk_t* chunk;

	// The current scope.
	compiler_t state;
} vm_compiler_t;

// compile_tree(vm_compiler_t*, parse_result_t*, bool) -> void
// Compiles the ast into a chunk of bytecode.
void compile_tree(vm_compiler_t* state, parse_result_t* result, bool terminate);

#endif /* compile_bytecode_h */

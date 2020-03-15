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
#include "../../../vm/bytecode.h"

// compile_tree(chunk_t*, parse_result_t*, bool) -> bool
// Compiles the ast into a chunk of bytecode.
bool compile_tree(chunk_t* chunk, parse_result_t* result, bool terminate);

#endif /* compile_bytecode_h */

//
// Curly
// gencode.h: Header file for gencode.c.
//
// jenra
// March 21 2020
//

#ifndef gencode_h
#define gencode_h

#include "../correctness/compiler_struct.h"
#include "combinator.h"

// convert_tree_ir(compiler_t*, parse_result_t*) -> void
// Converts the ast into ir.
void convert_tree_ir(compiler_t* state, parse_result_t* result);

#endif /* gencode_h */

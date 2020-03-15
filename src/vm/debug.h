//
// Curly
// debug.h: Header file for debug.c.
//
// jenra
// March 14 2020
//

#ifndef debug_h
#define debug_h

#include "bytecode.h"

// dis_opcode(chunk_t*, int, int*) -> int
// Disassembles a single opcode and returns the index offset.
int dis_opcode(chunk_t* chunk, int index, int* global_count);

// disassemble(chunk_t*, char*) -> void
// Disassembles a given chunk of bytecode.
void disassemble(chunk_t* chunk, char* name);

#endif /* debug_h */

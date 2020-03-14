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

// disassemble(chunk_t*, char*) -> void
// Disassembles a given chunk of bytecode.
void disassemble(chunk_t* chunk, char* name);

#endif /* debug_h */

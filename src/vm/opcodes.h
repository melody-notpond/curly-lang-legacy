//
// Curly
// opcodes.h: Header file for opcodes.c.
//
// jenra
// March 14 2020
//

#ifndef opcodes_h
#define opcodes_h

#include "vm.h"

// The jump table for the opcodes.
int (*opcode_funcs[256])(CurlyVM* vm, uint8_t opcode, uint8_t* pc);

// init_opcodes(void) -> void
// Initialises the jump table.
void init_opcodes();

#endif /* opcodes_h */

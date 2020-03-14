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

// Represents an opcode
enum
{
	// NOP - does nothing
	OPCODE_NOP				= 0b00000000,

	// BREAK - halts the virtual machine temporarily
	OPCODE_BREAK			= 0b00000001,

	// i64 VALUE - loads an i64 value onto the stack
	OPCODE_LOAD_I64			= 0b00000100,
	OPCODE_LOAD_I64_LONG	= 0b00000101,

	// f64 VALUE - loads a f64 value onto the stack
	OPCODE_LOAD_F64			= 0b00000110,
	OPCODE_LOAD_F64_LONG	= 0b00000111
};

// The jump table for the opcodes.
int (*opcode_funcs[256])(CurlyVM* vm, uint8_t opcode, uint8_t* pc);

// init_opcodes(void) -> void
// Initialises the jump table.
void init_opcodes();

#endif /* opcodes_h */

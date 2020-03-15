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
	OPCODE_NOP			= 0b00000000,

	// BREAK - halts the virtual machine temporarily
	OPCODE_BREAK		= 0b00000001,

	// i64 VALUE - loads an int value onto the stack
	// f64 VALUE - loads a double value onto the stack
	OPCODE_LOAD			= 0b00000010,
	OPCODE_LOAD_LONG	= 0b00000011,

	// MUL i64 i64 - multiplies two ints
	// MUL i64 f64 - multiplies an int and a double
	// MUL f64 i64 - multiplies a double and an int
	// MUL f64 f64 - multiplies two doubles
	OPCODE_MUL_I64_I64	= 0b00000100,
	OPCODE_MUL_I64_F64	= 0b00000101,
	OPCODE_MUL_F64_I64	= 0b00000110,
	OPCODE_MUL_F64_F64	= 0b00000111
};

// The jump table for the opcodes.
int (*opcode_funcs[256])(CurlyVM* vm, uint8_t opcode, uint8_t* pc);

// init_opcodes(void) -> void
// Initialises the jump table.
void init_opcodes();

#endif /* opcodes_h */

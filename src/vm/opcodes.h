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

	// OPNAME i64 i64 - multiplies two ints
	// OPNAME i64 f64 - multiplies an int and a double (returns int)
	// OPNAME f64 i64 - multiplies a double and an int (returns double)
	// OPNAME f64 f64 - multiplies two doubles
#define opcode_infix(opname, opcode)						\
	OPCODE_##opname##_I64_I64	= (opcode) | 0b00000000,	\
	OPCODE_##opname##_I64_F64	= (opcode) | 0b00000001,	\
	OPCODE_##opname##_F64_I64	= (opcode) | 0b00000010,	\
	OPCODE_##opname##_F64_F64	= (opcode) | 0b00000011

	opcode_infix(MUL, 0b00000100),
	opcode_infix(DIV, 0b00001000),
	opcode_infix(ADD, 0b00001100),
	opcode_infix(SUB, 0b00010000),

#undef opcode_infix

	// MOD i64 i64 - finds the remainder of the integer division
	OPCODE_MOD			= 0b00010100,


	// PRINT i64 - prints an integer
	// PRINT f64 - prints a double
	OPCODE_PRINT_I64	= 0b11111110,
	OPCODE_PRINT_F64	= 0b11111111
};

// The jump table for the opcodes.
int (*opcode_funcs[256])(CurlyVM* vm, uint8_t opcode, uint8_t* pc);

// init_opcodes(void) -> void
// Initialises the jump table.
void init_opcodes();

#endif /* opcodes_h */

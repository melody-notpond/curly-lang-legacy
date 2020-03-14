//
// Curly
// opcodes.c: Implements the opcodes of the virtual machine.
//
// jenra
// March 14 2020
//

#include "opcodes.h"
#include <stdio.h>

// The jump table for the opcodes.
int (*opcode_funcs[256])(CurlyVM* vm, uint8_t opcode, uint8_t* pc);

// UNKNOWN
// Implements an unknown opcode.
int opcode_unknown_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	printf("Unknown opcode: 0x%02X (ignored)\n", opcode);
	return 1;
}

// NOP
// Implements no op.
int opcode_nop_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	return 1;
}

// BREAK
// Implements break.
int opcode_break_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	vm->running = false;
	return 0;
}

// i64 VALUE, f64 VALUE
// Implements loading values onto the stack.
int opcode_load_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	printf("We're loading a value!! It's %lli\n", vm->chunk->pool.values[*(pc + 1)]);
	return 2;
}

// init_opcodes(void) -> void
// Initialises the jump table.
void init_opcodes()
{
	for (int i = 0; i < 256; i++)
	{
		opcode_funcs[i] = opcode_unknown_func;
	}

	opcode_funcs[OPCODE_NOP				] = opcode_nop_func;
	opcode_funcs[OPCODE_BREAK			] = opcode_break_func;
	opcode_funcs[OPCODE_LOAD_I64		] = opcode_load_func;
	opcode_funcs[OPCODE_LOAD_I64_LONG	] = opcode_load_func;
	opcode_funcs[OPCODE_LOAD_F64		] = opcode_load_func;
	opcode_funcs[OPCODE_LOAD_F64_LONG	] = opcode_load_func;
}

//
// Curly
// opcodes.c: Implements the opcodes of the virtual machine.
//
// jenra
// March 14 2020
//

#include "opcodes.h"

// The jump table for the opcodes.
int (*opcode_funcs[256])(CurlyVM* vm);

// UNKNOWN
// Implements an unknown opcode.
int unknown_opcode(CurlyVM* vm)
{
	printf("Unknown opcode: 0x%02X\n", *vm->pc);
	return 1;
}

// init_opcodes(void) -> void
// Initialises the jump table.
void init_opcodes()
{
	for (int i = 0; i < 256; i++)
	{
		opcode_funcs[i] = unknown_opcode;
	}
}

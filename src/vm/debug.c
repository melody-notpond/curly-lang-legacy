//
// Curly
// debug.c: Implements various debugging utilities for the vm and assembler.
//
// jenra
// March 14 2020
//

#include <stdio.h>
#include "debug.h"

int simple_opcode(char* name)
{
	printf("%s\n", name);
	return 1;
}

// dis_opcode(chunk_t*, int) -> int
// Disassembles a single opcode and returns the index offset.
int dis_opcode(chunk_t* chunk, int index)
{
	// Index
	printf("%04X  ", index);

	// Get next opcode and disassemble it
	uint8_t opcode = chunk->bytes[index];
	switch(opcode)
	{
		case OPCODE_NOP:
			// NOP
			return simple_opcode("NOP");
		case OPCODE_BREAK:
			// BREAK
			return simple_opcode("BREAK");
		default:
			// UNKNOWN (0xFF)
			printf("UNKNOWN (0x%02X)\n", opcode);
			return 1;
	}
}

// disassemble(chunk_t*, char*) -> void
// Disassembles a given chunk of bytecode.
void disassemble(chunk_t* chunk, char* name)
{
	// Header
	printf("== %s ==\n", name);

	for (int i = 0; i < chunk->count;)
	{
		// Disassemble each opcode
		i += dis_opcode(chunk, i);
	}
}

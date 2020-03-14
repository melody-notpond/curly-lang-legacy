//
// Curly
// debug.c: Implements various debugging utilities for the vm and assembler.
//
// jenra
// March 14 2020
//

#include <stdbool.h>
#include <stdio.h>
#include "debug.h"

// simple_opcode(char*) -> int
// Disassembles a simple opcode.
int simple_opcode(char* name)
{
	printf("%s\n", name);
	return 1;
}

// load_opcode(char*, int, chunk_t*, uint8_t) -> int
// Disassembles a loading instruction.
int load_opcode(char* name, int index, chunk_t* chunk, uint8_t opcode)
{
	printf("%s ", name);
	bool long_op = opcode & 1;
	int pool_index = chunk->bytes[index + 1];

	if (long_op)
	{
		pool_index |= chunk->bytes[index + 2] <<  8;
		pool_index |= chunk->bytes[index + 3] << 16;
		printf("0x%06X (", pool_index);
	} else
		printf("0x%02X (", pool_index);

	if (opcode & 2)
	{
		union
		{
			double f64;
			int64_t i64;
		} int_to_double;
		int_to_double.i64 = chunk->pool.values[pool_index];
		printf("%f)\n", int_to_double.f64);
	} else
		printf("%lli)\n", chunk->pool.values[pool_index]);

	return long_op ? 4 : 2;
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
		case OPCODE_LOAD_I64:
		case OPCODE_LOAD_I64_LONG:
			// i64 VALUE
			return load_opcode("i64", index, chunk, opcode);
		case OPCODE_LOAD_F64:
		case OPCODE_LOAD_F64_LONG:
			// f64 VALUE
			return load_opcode("f64", index, chunk, opcode);
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

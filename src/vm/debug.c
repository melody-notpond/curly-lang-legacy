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
#include "opcodes.h"
#include "types.h"

// smart_print_value(cvalue_t) -> void
// Prints out a cnumb_t as an int or double depending on which is more likely
void smart_print_value(chunk_t* chunk, cvalue_t value)
{
	// If it matches a pointer in the string pool, it's probably a string
	for (int i = 0; i < chunk->strs.count; i++)
	{
		if (value.str == chunk->strs.values[i].str)
		{
			printf("\"%s\"", value.str);
			return;
		}
	}

	// See IEEE standard if this makes no sense
	int exponent = (value.i64 >> 52 & 0b011111111111) - 1023;

	// -20 <= exponent <= 20 is a good range for doubles you'd actually use
	// This is a debugging tool, so it doesn't have to be perfect
	if (-20 <= exponent && exponent <= 20)
		printf("%f", value.f64);
	else
		printf("%lli", value.i64);
}

// simple_opcode(char*) -> int
// Disassembles a simple opcode.
int simple_opcode(char* name)
{
	printf("%s\n", name);
	return 1;
}

// load_opcode(char*, int, chunk_t*, uint8_t) -> int
// Disassembles a loading opcode.
int load_opcode(char* name, int index, chunk_t* chunk, uint8_t opcode)
{
	// Print out the name
	printf("%s ", name);

	// Get the pool index
	bool long_op = opcode & 1;
	int pool_index = chunk->bytes[index + 1];
	if (long_op)
	{
		pool_index |= chunk->bytes[index + 2] <<  8;
		pool_index |= chunk->bytes[index + 3] << 16;
	}

	// Print out the value
	printf("0x%06X (", pool_index);
	cvalue_t value = chunk->pool.values[pool_index];
	smart_print_value(chunk, value);
	puts(")");
	return long_op ? 4 : 2;
}

// infix_opcode(char*, uint8_t) -> int
// Disassembles an infix opcode.
int infix_opcode(char* name, uint8_t opcode)
{
	printf("%s %c64 %c64\n", name, (opcode & 2 ? 'f' : 'i'), (opcode & 1 ? 'f' : 'i'));
	return 1;
}

// set_global_opcode(char*, chunk_t*, int*) -> int
// Disassembles a set global opcode.
int set_global_opcode(char* name, chunk_t* chunk, int* global_count)
{
	printf("%s %s\n", name, chunk->globals.names[(*global_count)++]);
	return 1;
}

// global_opcode(char*, int, chunk_t*, uint8_t) -> int
// Disassembles a global opcode.
int global_opcode(char* name, int index, chunk_t* chunk, uint8_t opcode)
{
	// Print out the name
	printf("%s ", name);

	// Get the global index
	bool long_op = opcode & 1;
	int globals_index = chunk->bytes[index + 1];
	if (long_op)
	{
		globals_index |= chunk->bytes[index + 2] <<  8;
		globals_index |= chunk->bytes[index + 3] << 16;
	}

	// Print out the name
	printf("0x%06X (%s)\n", globals_index, chunk->globals.names[globals_index]);
	return long_op ? 4 : 2;
}

// dis_opcode(chunk_t*, int, int*) -> int
// Disassembles a single opcode and returns the index offset.
int dis_opcode(chunk_t* chunk, int index, int* global_count)
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
		case OPCODE_LOAD:
		case OPCODE_LOAD_LONG:
			// i64 VALUE
			// f64 VALUE
			return load_opcode("LOAD", index, chunk, opcode);
		case OPCODE_MUL_I64_I64:
		case OPCODE_MUL_I64_F64:
		case OPCODE_MUL_F64_I64:
		case OPCODE_MUL_F64_F64:
			// MUL i64 i64
			// MUL i64 f64
			// MUL f64 i64
			// MUL f64 f64
			return infix_opcode("MUL", opcode);
		case OPCODE_DIV_I64_I64:
		case OPCODE_DIV_I64_F64:
		case OPCODE_DIV_F64_I64:
		case OPCODE_DIV_F64_F64:
			// DIV i64 i64
			// DIV i64 f64
			// DIV f64 i64
			// DIV f64 f64
			return infix_opcode("DIV", opcode);
		case OPCODE_ADD_I64_I64:
		case OPCODE_ADD_I64_F64:
		case OPCODE_ADD_F64_I64:
		case OPCODE_ADD_F64_F64:
			// ADD i64 i64
			// ADD i64 f64
			// ADD f64 i64
			// ADD f64 f64
			return infix_opcode("ADD", opcode);
		case OPCODE_SUB_I64_I64:
		case OPCODE_SUB_I64_F64:
		case OPCODE_SUB_F64_I64:
		case OPCODE_SUB_F64_F64:
			// SUB i64 i64
			// SUB i64 f64
			// SUB f64 i64
			// SUB f64 f64
			return infix_opcode("SUB", opcode);
		case OPCODE_MOD:
			// MOD i64 i64
			return infix_opcode("MOD", opcode);
		case OPCODE_SET_GLOBAL:
			return set_global_opcode("GLOBAL SET", chunk, global_count);
		case OPCODE_GLOBAL:
		case OPCODE_GLOBAL_LONG:
			return global_opcode("GLOBAL", index, chunk, opcode);
		case OPCODE_POP:
			puts("POP");
			return 1;
		case OPCODE_PRINT_STR:
			// PRINT str
			puts("PRINT str");
			return 1;
		case OPCODE_PRINT_I64:
			// PRINT i64
			puts("PRINT i64");
			return 1;
		case OPCODE_PRINT_F64:
			// PRINT f64
			puts("PRINT f64");
			return 1;
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
	int global_count = 0;

	for (int i = 0; i < chunk->count;)
	{
		// Disassemble each opcode
		i += dis_opcode(chunk, i, &global_count);
	}
}

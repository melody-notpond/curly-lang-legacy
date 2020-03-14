//
// Curly
// bytecode.h: Header file for bytecode.c.
//
// jenra
// March 14 2020
//

#ifndef bytecode_h
#define bytecode_h

#include <stdlib.h>

// Represents an opcode
enum
{
	// NOP - does nothing
	OPCODE_NOP,

	// BREAK - halts the virtual machine temporarily
	OPCODE_BREAK,

	// i32 VALUE - loads an i32 value onto the stack
	OPCODE_LOAD_I32,

	// i64 VALUE - loads an i64 value onto the stack
	OPCODE_LOAD_I64,

	// f32 VALUE - loads a f32 value onto the stack
	OPCODE_LOAD_F32,

	// f64 VALUE - loads a f64 value onto the stack
	OPCODE_LOAD_F64
};

// Represents a chunk of bytecode
typedef struct
{
	// The bytecode itself
	uint8_t* bytes;

	// The amount of memory allocated for the list
	size_t size;

	// The number of occupied bytes
	size_t count;
} chunk_t;

// init_chunk(void) -> chunk_t
// Initialises an empty chunk of bytecode.
chunk_t init_chunk();

// write_chunk(chunk_t*, uint8_t) -> void
// Writes a single byte to a chunk.
void write_chunk(chunk_t* chunk, uint8_t value);

// clean_chunk(chunk_t*) -> void
// Cleans up a chunk of bytecode.
void clean_chunk(chunk_t* chunk);

#endif /* bytecode_h */

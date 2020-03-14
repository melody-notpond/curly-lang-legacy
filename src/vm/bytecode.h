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

// Represents a list of values referenced by the program.
struct s_values
{
	// The list of values
	uint64_t* values;

	// The amount of memory allocated for the list
	int size;

	// The number of values occupied
	int count;
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

	// The constant pool
	struct s_values pool;
} chunk_t;

// init_chunk(void) -> chunk_t
// Initialises an empty chunk of bytecode.
chunk_t init_chunk();

// write_chunk(chunk_t*, uint8_t) -> void
// Writes a single byte to a chunk.
void write_chunk(chunk_t* chunk, uint8_t value);

// add_i64(chunk_t*, int64_t) -> void
// Adds a 64 bit int to the constant pool.
void add_i64(chunk_t* chunk, int64_t value);

// add_f64(chunk_t*, int64_t) -> void
// Adds a double to the constant pool.
void add_f64(chunk_t* chunk, double value);

// clean_chunk(chunk_t*) -> void
// Cleans up a chunk of bytecode.
void clean_chunk(chunk_t* chunk);

#endif /* bytecode_h */

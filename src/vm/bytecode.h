//
// Curly
// bytecode.h: Header file for bytecode.c.
//
// jenra
// March 14 2020
//

#ifndef bytecode_h
#define bytecode_h

#include <stdbool.h>
#include <stdlib.h>

#include "types.h"

// Represents a list of values referenced by the program.
struct s_values
{
	// The list of values.
	cvalue_t* values;

	// The amount of memory allocated for the list.
	int size;

	// The number of values occupied.
	int count;
};

// Represents a list of globals active in the program.
typedef struct
{
	// The names of the global.
	char** names;

	// The amount of memory allocated for the list.
	int size;

	// The number of values occupied.
	int count;
} globals_t;

// Represents a local scope.
struct s_chunk_scope
{
	// The number of items on the stack above the set of locals
	int stack_count;

	// The last scope called
	struct s_chunk_scope* last;
};

// Represents a chunk of bytecode.
typedef struct
{
	// The bytecode itself.
	uint8_t* bytes;

	// The amount of memory allocated for the list.
	size_t size;

	// The number of occupied bytes.
	size_t count;

	// The constant pool.
	struct s_values pool;

	// The string pool;
	struct s_values strs;

	// The globals.
	globals_t globals;

	// The current scope.
	// This is used during compilation to put the right opcodes for locals.
	struct s_chunk_scope* scope;
} chunk_t;

// init_chunk(void) -> chunk_t
// Initialises an empty chunk of bytecode.
chunk_t init_chunk();

// chunk_global(chunk_t*, char*) -> void
// Adds a global to the list of globals.
void chunk_global(chunk_t* chunk, char* name);

// chunk_get_local(chunk_t*, int, int) -> void
// Writes the appropriate instruction to copy a local to the top of the stack.
void chunk_get_local(chunk_t* chunk, int depth, int index);

// chunk_add_string(chunk_t*, char*) -> void
// Adds a string to the constant pool.
void chunk_add_string(chunk_t* chunk, char* string);

// chunk_add_i64(chunk_t*, int64_t) -> void
// Adds a 64 bit int to the constant pool.
void chunk_add_i64(chunk_t* chunk, int64_t value);

// chunk_add_f64(chunk_t*, int64_t) -> void
// Adds a double to the constant pool.
void chunk_add_f64(chunk_t* chunk, double value);

// chunk_push_scope(chunk_t*) -> void
// Pushes a new local scope onto the stack of scopes.
void chunk_push_scope(chunk_t* chunk);

// chunk_pop_scope(chunk_t*) -> bool
// Pops a local scope from the stack of scopes. Returns true if a scope was popped.
bool chunk_pop_scope(chunk_t* chunk);

// chunk_opcode(chunk_t*, uint8_t) -> void
// Writes an opcode to a chunk of bytecode.
void chunk_opcode(chunk_t* chunk, uint8_t opcode);

// clean_chunk(chunk_t*, bool) -> void
// Cleans up a chunk of bytecode.
void clean_chunk(chunk_t* chunk, bool clear_globals);

#endif /* bytecode_h */

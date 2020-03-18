//
// Curly
// bytecode.c: Implements various utilities relating to bytecode.
//
// jenra
// March 14 2020
//

#include <string.h>

#include "bytecode.h"
#include "opcodes.h"
#include "types.h"

// init_chunk(void) -> chunk_t
// Initialises an empty chunk of bytecode.
chunk_t init_chunk()
{
	chunk_t chunk;
	chunk.bytes = NULL;
	chunk.size = 0;
	chunk.count = 0;
	chunk.pool.values = NULL;
	chunk.pool.size = 0;
	chunk.pool.count = 0;
	chunk.strs.values = NULL;
	chunk.strs.size = 0;
	chunk.strs.count = 0;
	chunk.globals.names = NULL;
	chunk.globals.size = 0;
	chunk.globals.count = 0;
	chunk.scope = NULL;
	return chunk;
}

// append_element(void*, size_t&, size_t&, type, type_t) -> void
// Appends an element to a list.
#define append_element(values, count, size, type, value) do				\
{																		\
	/* Resize the list if necessary */									\
	if (size == 0)														\
		values = calloc((size = 8), sizeof(type));						\
	else if (count >= size)												\
		values = realloc(values, (size <<= 1) * sizeof(type));			\
																		\
	/* Append the value */												\
	values[count++] = value;											\
} while (0)

// write_chunk(chunk_t*, uint8_t) -> void
// Writes a single byte to a chunk.
void write_chunk(chunk_t* chunk, uint8_t value)
{
	// Append the byte to the chunk
	append_element(chunk->bytes, chunk->count, chunk->size, uint8_t, value);
}

// chunk_add_constant(chunk_t*, cvalue_t) -> void
// Adds a constant value to the constant pool.
void chunk_add_constant(chunk_t* chunk, cvalue_t value)
{
	// Search for the constant
	int index;
	struct s_values* pool = &chunk->pool;
	for (index = 0; index < pool->count; index++)
	{
		if (value.i64 == pool->values[index].i64)
			break;
	}

	// If it doesn't exist, append it to the pool
	if (index == pool->count)
		append_element(pool->values, pool->count, pool->size, cvalue_t, value);

	// Store the appropriate instruction
	if (index <= 0xFF)
	{
		write_chunk(chunk, OPCODE_LOAD);
		write_chunk(chunk, index);
	} else
	{
		write_chunk(chunk, OPCODE_LOAD_LONG);
		write_chunk(chunk, (index      ) & 0xFF);
		write_chunk(chunk, (index >>  8) & 0xFF);
		write_chunk(chunk, (index >> 16) & 0xFF);
	}
}

// chunk_add_i64(chunk_t*, int64_t) -> void
// Adds a 64 bit int to the constant pool.
void chunk_add_i64(chunk_t* chunk, int64_t value)
{
	cvalue_t boxed;
	boxed.i64 = value;
	chunk_add_constant(chunk, boxed);
}

// chunk_add_f64(chunk_t*, int64_t) -> void
// Adds a double to the constant pool.
void chunk_add_f64(chunk_t* chunk, double value)
{
	cvalue_t boxed;
	boxed.f64 = value;
	chunk_add_constant(chunk, boxed);
}

// chunk_add_string(chunk_t*, char*) -> void
// Adds a string to the constant pool.
void chunk_add_string(chunk_t* chunk, char* string)
{
	// Search for the string
	cvalue_t boxed;
	int index;
	struct s_values* strings = &chunk->strs;
	for (index = 0; index < strings->count; index++)
	{
		boxed = strings->values[index];
		if (!strcmp(string, boxed.str))
			break;
	}

	// If the string isn't stored, add it
	if (index == strings->count)
	{
		boxed.str = strdup(string);
		append_element(strings->values, strings->count, strings->size, cvalue_t, boxed);
	}

	// Generate the constant loading stuff
	chunk_add_constant(chunk, boxed);
}

// chunk_global(chunk_t*, char*) -> void
// Adds a global to the list of globals.
void chunk_global(chunk_t* chunk, char* name)
{
	// Search for the global
	int index;
	globals_t* globals = &chunk->globals;
	for (index = 0; index < globals->count; index++)
	{
		if (!strcmp(name, globals->names[index]))
			break;
	}

	if (index == globals->count)
	{
		// If it doesn't exist, append it to the list of globals and create the global
		append_element(globals->names, globals->count, globals->size, char*, strdup(name));
		write_chunk(chunk, OPCODE_SET_GLOBAL);

	// Store the appropriate load instruction
	} else if (index <= 0xFF)
	{
		write_chunk(chunk, OPCODE_GLOBAL);
		write_chunk(chunk, index);
	} else
	{
		write_chunk(chunk, OPCODE_GLOBAL_LONG);
		write_chunk(chunk, (index      ) & 0xFF);
		write_chunk(chunk, (index >>  8) & 0xFF);
		write_chunk(chunk, (index >> 16) & 0xFF);
	}
}

#undef append_element

// push_scope(chunk_t*) -> void
// Pushes a new local scope onto the stack of scopes.
void push_scope(chunk_t* chunk)
{
	struct s_chunk_scope* scope = malloc(sizeof(struct s_chunk_scope));
	scope->stack_count = 0;
	scope->last = chunk->scope;
	chunk->scope = scope;
}

// pop_scope(chunk_t*) -> bool
// Pops a local scope from the stack of scopes. Returns true if a scope was popped.
bool pop_scope(chunk_t* chunk)
{
	if (chunk->scope == NULL)
		return false;

	struct s_chunk_scope* scope = chunk->scope;
	chunk->scope = scope->last;
	free(scope);
	return true;
}

// clean_chunk(chunk_t*, bool) -> void
// Cleans up a chunk of bytecode.
void clean_chunk(chunk_t* chunk, bool clear_globals)
{
	if (chunk == NULL)
		return;

	free(chunk->bytes);
	chunk->bytes = NULL;
	chunk->size = 0;
	chunk->count = 0;
	free(chunk->pool.values);
	chunk->pool.values = NULL;
	chunk->pool.size = 0;
	chunk->pool.count = 0;
	free(chunk->strs.values);
	chunk->strs.values = NULL;
	chunk->strs.size = 0;
	chunk->strs.count = 0;

	if (clear_globals)
	{
		for (int i = 0; i < chunk->globals.count; i++)
		{
			free(chunk->globals.names[i]);
		}
		free(chunk->globals.names);
		chunk->globals.names = NULL;
		chunk->globals.size = 0;
		chunk->globals.count = 0;
	}

	while (pop_scope(chunk));
}

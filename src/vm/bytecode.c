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

		// Update the number of items on the stack
		if (chunk->scope != NULL)
			chunk->scope->stack_count--;
	} else
	{
		// Store the appropriate load instruction
		if (index <= 0xFF)
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

		// Update the number of items on the stack
		if (chunk->scope != NULL)
			chunk->scope->stack_count++;
	}
}
#include <stdio.h>

// get_stack_offset(struct s_chunk_scope*, int, int) -> int
// Calculates the offset between the top of stack and the local desired.
int get_stack_offset(struct s_chunk_scope* scope, int depth, int index)
{
	int total = -index;

	// Iterate over every scope up to the depth specified and find the stack difference
	while (1 + depth-- && scope != NULL)
	{
		total += scope->stack_count;
		scope = scope->last;
	}

	return total;
}

// chunk_get_local(chunk_t*, int, int) -> void
// Writes the appropriate instruction to copy a local to the top of the stack.
void chunk_get_local(chunk_t* chunk, int depth, int index)
{
	int total = get_stack_offset(chunk->scope, depth, index);

	// Store the appropriate load instruction
	if (total < 256)
	{
		write_chunk(chunk, OPCODE_LOCAL);
		write_chunk(chunk, total);
	} else
	{
		write_chunk(chunk, OPCODE_LOCAL_LONG);
		write_chunk(chunk, (total      ) & 0xFF);
		write_chunk(chunk, (total >>  8) & 0xFF);
		write_chunk(chunk, (total >> 16) & 0xFF);
	}

	// Update the number of items on the stack
	if (chunk->scope != NULL)
		chunk->scope->stack_count++;
}

// chunk_set_local(chunk_t*, int, int) -> void
// Writes the appropriate instruction to set a local's value.
void chunk_set_local(chunk_t* chunk, int depth, int index)
{
	int total = get_stack_offset(chunk->scope, depth, index);

	// Store the appropriate load instruction
	if (total < 256)
	{
		write_chunk(chunk, OPCODE_SET_LOCAL);
		write_chunk(chunk, total);
	} else
	{
		write_chunk(chunk, OPCODE_SET_LOCAL_LONG);
		write_chunk(chunk, (total      ) & 0xFF);
		write_chunk(chunk, (total >>  8) & 0xFF);
		write_chunk(chunk, (total >> 16) & 0xFF);
	}

	// Update the number of items on the stack
	if (chunk->scope != NULL)
		chunk->scope->stack_count++;
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

	// Update the number of items on the stack
	if (chunk->scope != NULL)
		chunk->scope->stack_count++;
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

#undef append_element

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

// chunk_push_scope(chunk_t*) -> void
// Pushes a new local scope onto the stack of scopes.
void chunk_push_scope(chunk_t* chunk)
{
	struct s_chunk_scope* scope = malloc(sizeof(struct s_chunk_scope));
	scope->stack_count = 0;
	scope->last = chunk->scope;
	chunk->scope = scope;
}

// chunk_pop_scope(chunk_t*) -> bool
// Pops a local scope from the stack of scopes. Returns true if a scope was popped.
bool chunk_pop_scope(chunk_t* chunk)
{
	// Don't do anything if it's empty
	if (chunk->scope == NULL)
		return false;

	// Store the appropriate instruction
	struct s_chunk_scope* scope = chunk->scope;
	int offset = scope->stack_count - 1;
	if (scope->stack_count <= 0xFF)
	{
		write_chunk(chunk, OPCODE_POP_SCOPE);
		write_chunk(chunk, offset);
	} else
	{
		write_chunk(chunk, OPCODE_POP_SCOPE_LONG);
		write_chunk(chunk, (offset      ) & 0xFF);
		write_chunk(chunk, (offset >>  8) & 0xFF);
	}

	// Free the scope
	chunk->scope = scope->last;
	free(scope);

	// Update the number of items on the stack
	if (chunk->scope != NULL)
		chunk->scope->stack_count++;
	return true;
}

// chunk_opcode(chunk_t*, uint8_t) -> void
// Writes an opcode to a chunk of bytecode.
void chunk_opcode(chunk_t* chunk, uint8_t opcode)
{
	if (chunk->scope != NULL)
	{
		// These instructions remove 1 from the stack
		if ((OPCODE_MUL_I64_I64 <= opcode && opcode <= OPCODE_MOD)
		 || opcode == OPCODE_POP)
			chunk->scope->stack_count--;
	}

	// Write the opcode
	write_chunk(chunk, opcode);
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

	while (chunk_pop_scope(chunk));
}

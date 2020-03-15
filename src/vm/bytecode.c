//
// Curly
// bytecode.c: Implements various utilities relating to bytecode.
//
// jenra
// March 14 2020
//

#include "bytecode.h"
#include "opcodes.h"

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
	return chunk;
}

// append_element(void*, size_t&, size_t&, type, type_t) -> void
// Appends an element to a list.
#define append_element(values, count, size, type, value) do				\
{																		\
	/* Resize the list if necessary */									\
	if (size == 0)														\
		values = malloc((size = 8) * sizeof(type));						\
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

// chunk_add_i64(chunk_t*, int64_t) -> void
// Adds a 64 bit int to the constant pool.
void chunk_add_i64(chunk_t* chunk, int64_t value)
{
	// Search for the constant
	int index;
	struct s_values* pool = &chunk->pool;
	for (index = 0; index < pool->count; index++)
	{
		if (value == pool->values[index])
			break;
	}

	// If it doesn't exist, append it to the pool
	if (index == pool->count)
		append_element(pool->values, pool->count, pool->size, int64_t, value);

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

#undef append_element

// chunk_add_f64(chunk_t*, int64_t) -> void
// Adds a double to the constant pool.
void chunk_add_f64(chunk_t* chunk, double value)
{
	// This union gets the double's binary representation
	union
	{
		double f64;
		int64_t i64;
	} double_to_int;
	double_to_int.f64 = value;

	chunk_add_i64(chunk, double_to_int.i64);
}

// clean_chunk(chunk_t*) -> void
// Cleans up a chunk of bytecode.
void clean_chunk(chunk_t* chunk)
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
}

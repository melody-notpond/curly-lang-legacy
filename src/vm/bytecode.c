//
// Curly
// bytecode.c: Implements various utilities relating to bytecode.
//
// jenra
// March 14 2020
//

#include "bytecode.h"

// init_chunk(void) -> chunk_t
// Initialises an empty chunk of bytecode.
chunk_t init_chunk()
{
	chunk_t chunk;
	chunk.bytes = NULL;
	chunk.size = 0;
	chunk.count = 0;
	return chunk;
}

// write_chunk(chunk_t*, int8_t) -> void
// Writes a single byte to a chunk.
void write_chunk(chunk_t* chunk, int8_t value)
{
	// Resize if necessary
	if (chunk->size == 0)
		chunk->bytes = malloc(chunk->size = 8);
	else if (chunk->count >= chunk->size)
		chunk->bytes = realloc(chunk->bytes, (chunk->size <<= 1));
	
	// Append the byte
	chunk->bytes[chunk->count++] = value;
}

// clean_chunk(chunk_t*) -> void
// Cleans up a chunk of bytecode.
void clean_chunk(chunk_t* chunk)
{
	free(chunk->bytes);
	chunk->bytes = NULL;
	chunk->size = 0;
	chunk->count = 0;
}

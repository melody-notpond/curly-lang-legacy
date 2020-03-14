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

// clean_chunk(chunk_t*) -> void
// Cleans up a chunk of bytecode.
void clean_chunk(chunk_t* chunk)
{
	free(chunk->bytes);
	chunk->bytes = NULL;
	chunk->size = 0;
	chunk->count = 0;
}

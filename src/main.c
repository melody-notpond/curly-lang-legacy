//
// Curly
// main.c: Runs the command line interface.
//
// jenra
// March 3 2020
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm/debug.h"

int main(int argc, char** argv)
{
	chunk_t chunk = init_chunk();
	for (int i = 0; i < 259; i++)
		add_i64(&chunk, i);
	write_chunk(&chunk, OPCODE_NOP);
	write_chunk(&chunk, OPCODE_BREAK);
	disassemble(&chunk, "test.o");
	clean_chunk(&chunk);
}

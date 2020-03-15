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
#include "vm/opcodes.h"
#include "vm/vm.h"

int main(int argc, char** argv)
{
	chunk_t chunk = init_chunk();
	chunk_add_i64(&chunk, -5);
	chunk_add_f64(&chunk, 3.14159);
	chunk_add_i64(&chunk, 42);
	for (int i = 0; i < 256; i++)
		chunk_add_i64(&chunk, i);
	write_chunk(&chunk, OPCODE_BREAK);
	
	disassemble(&chunk, "test.o");
	puts("\n");

	CurlyVM vm;
	init_vm(&vm, &chunk);

	vm_run(&vm);

	clean_vm(&vm);
}

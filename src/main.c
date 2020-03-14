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

#include "vm/vm.h"
#include "vm/debug.h"

int main(int argc, char** argv)
{
	chunk_t chunk = init_chunk();
	chunk_add_i64(&chunk, -5);
	write_chunk(&chunk, OPCODE_BREAK);

	CurlyVM vm;
	init_vm(&vm, &chunk);

	vm_run(&vm);

	disassemble(&chunk, "test.o");
	clean_vm(&vm);
}

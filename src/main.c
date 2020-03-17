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
	chunk_add_i64(&chunk, -5); // something not in the scope

	chunk_add_i64(&chunk, 1); // local variables
	chunk_add_i64(&chunk, 2);
	chunk_add_i64(&chunk, 3);
	chunk_add_i64(&chunk, 4); // end of local variables

	chunk_add_i64(&chunk, 10);
	write_chunk(&chunk, OPCODE_COPY_STACK);
	write_chunk(&chunk, 3);
	write_chunk(&chunk, OPCODE_ADD_I64_I64); // some operation in the local scope

	write_chunk(&chunk, OPCODE_POP_SCOPE); // now we pop the values
	write_chunk(&chunk, 4);

	write_chunk(&chunk, OPCODE_BREAK);

	disassemble(&chunk, "test.o");
	puts("\n");

	CurlyVM vm;
	init_vm(&vm);
	vm_load(&vm, &chunk);

	vm.running = true;
	while (vm.running)
	{
		vm_stepi(&vm);
		printf("Stack: %lli %lli %lli %lli %lli %lli %lli %lli %lli %lli, sp = %i\n",
			vm.stack[0].i64,
			vm.stack[1].i64,
			vm.stack[2].i64,
			vm.stack[3].i64,
			vm.stack[4].i64,
			vm.stack[5].i64,
			vm.stack[6].i64,
			vm.stack[7].i64,
			vm.stack[8].i64,
			vm.stack[9].i64,
			(int) (vm.tos - vm.stack)
		);
	}

	clean_vm(&vm);
}

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

	chunk_push_scope(&chunk);
	chunk_add_i64(&chunk, 1); // local variables
	chunk_add_i64(&chunk, 2);
	chunk_add_i64(&chunk, 3);
	chunk_add_i64(&chunk, 4); // end of local variables

	chunk_add_i64(&chunk, 10);
	chunk_get_local(&chunk, 0, 2); // get the 3
	chunk_get_local(&chunk, 0, 2); // get the 3 again
	chunk_get_local(&chunk, 0, 0); // get the 1
	chunk_opcode(&chunk, OPCODE_ADD_I64_I64); // some operation in the local scope
	chunk_opcode(&chunk, OPCODE_ADD_I64_I64);
	chunk_opcode(&chunk, OPCODE_ADD_I64_I64);

	chunk_set_local(&chunk, 0, 1); // change the 1 into whatever the above value is
	chunk_opcode(&chunk, OPCODE_POP);

	chunk_opcode(&chunk, OPCODE_BREAK);

	disassemble(&chunk, "test.o");
	puts("\n");

	CurlyVM vm;
	init_vm(&vm);
	vm_load(&vm, &chunk);

	vm.running = true;
	while (vm.running)
	{
		vm_stepi(&vm);
		printf("Stack: %i %i %i %i %i %i %i %i %i %i %i %i %i %i, sp = %i\n",
			(char) vm.stack[ 0].i64,
			(char) vm.stack[ 1].i64,
			(char) vm.stack[ 2].i64,
			(char) vm.stack[ 3].i64,
			(char) vm.stack[ 4].i64,
			(char) vm.stack[ 5].i64,
			(char) vm.stack[ 6].i64,
			(char) vm.stack[ 7].i64,
			(char) vm.stack[ 8].i64,
			(char) vm.stack[ 9].i64,
			(char) vm.stack[10].i64,
			(char) vm.stack[11].i64,
			(char) vm.stack[12].i64,
			(char) vm.stack[13].i64,
			(int) (vm.tos - vm.stack)
		);
	}

	clean_vm(&vm);
}

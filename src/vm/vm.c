//
// Curly
// vm.c: Implements the virtual machine.
//
// jenra
// March 14 2020
//

#include "vm.h"

// init_vm(CurlyVM*) -> void
// Initialises a vm.
void init_vm(CurlyVM* vm)
{
	vm->chunk = NULL;
	vm->pc = NULL;

	if (opcode_funcs[0] == NULL)
		init_opcodes();
}

// stepi(CurlyVM*) -> void
// Steps the vm one instruction.
void stepi(CurlyVM* vm)
{
	vm->pc += opcode_funcs[*vm->pc](vm);
}

// clean_vm(CurlyVM*) -> void
// Cleans up a vm.
void clean_vm(CurlyVM* vm)
{
	clean_chunk(vm->chunk);
	vm->chunk = NULL;
	vm->pc = NULL;
}

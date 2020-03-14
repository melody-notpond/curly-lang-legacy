//
// Curly
// vm.c: Implements the virtual machine.
//
// jenra
// March 14 2020
//

#include "opcodes.h"
#include "vm.h"

// init_vm(CurlyVM*, chunk_t*) -> void
// Initialises a vm.
void init_vm(CurlyVM* vm, chunk_t* chunk)
{
	vm->chunk = chunk;
	vm->pc = chunk->bytes;

	// Initialise jump table if not done so already.
	if (opcode_funcs[0] == NULL)
		init_opcodes();
}

#define step_macro(vm, pc) do						\
{													\
	uint8_t opcode = *pc;							\
	pc += opcode_funcs[opcode](vm, opcode, pc);		\
} while (0)

// vm_stepi(CurlyVM*) -> void
// Steps the vm one instruction.
void vm_stepi(CurlyVM* vm)
{
	step_macro(vm, vm->pc);
}

// vm_run(CurlyVM*) -> void
// Runs the virtual machine.
void vm_run(CurlyVM* vm)
{
	uint8_t* pc = vm->pc;

	while (vm->running)
	{
		step_macro(vm, pc);
	}
	
	vm->pc = pc;
}

#undef step_macro

// clean_vm(CurlyVM*) -> void
// Cleans up a vm.
void clean_vm(CurlyVM* vm)
{
	if (vm == NULL)
		return;

	clean_chunk(vm->chunk);
	vm->chunk = NULL;
	vm->pc = NULL;
}

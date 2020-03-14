//
// Curly
// vm.c: Implements the virtual machine.
//
// jenra
// March 14 2020
//

#include "opcodes.h"
#include "vm.h"

// init_vm(CurlyVM*) -> void
// Initialises a vm.
void init_vm(CurlyVM* vm)
{
	vm->chunk = NULL;
	vm->pc = NULL;

	// Initialise jump table if not done so already.
	if (opcode_funcs[0] == NULL)
		init_opcodes();
}

#define step_macro(vm, pc) do						\
{													\
	uint8_t opcode = *pc;							\
	pc += opcode_funcs[opcode](vm, opcode, pc);		\
} while (0)

// stepi(CurlyVM*) -> void
// Steps the vm one instruction.
void stepi(CurlyVM* vm)
{
	step_macro(vm, vm->pc);
}

// run(CurlyVM*) -> void
// Runs the virtual machine.
void run(CurlyVM* vm)
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
	clean_chunk(vm->chunk);
	vm->chunk = NULL;
	vm->pc = NULL;
}

//
// Curly
// vm.c: Implements the virtual machine.
//
// jenra
// March 14 2020
//

#include <stdio.h>

#include "opcodes.h"
#include "vm.h"

// init_vm(CurlyVM*, chunk_t*) -> void
// Initialises a vm.
void init_vm(CurlyVM* vm, chunk_t* chunk)
{
	vm->chunk = chunk;
	vm->pc = chunk->bytes;
	vm->stack_size = 256;
	vm->stack = calloc(vm->stack_size, 8);
	vm->tos = vm->stack;
	vm->globals = NULL;
	vm->globals_count = 0;
	vm->globals_size = 0;
	vm->running = false;

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
	vm->running = true;

	while (vm->running)
	{
		step_macro(vm, pc);
	}

	vm->pc = pc;
}

#undef step_macro

// vm_push(CurlyVM*, cvalue_t) -> void
// Pushes a value onto the stack.
void vm_push(CurlyVM* vm, cvalue_t value)
{
	// Resize if necessary
	if (vm->stack + vm->stack_size <= vm->tos)
		vm->stack = realloc(vm->stack, (vm->stack_size <<= 1) << 3);

	// Append to stack
	*vm->tos++ = value;
}

// vm_pop(CurlyVM*) -> cvalue_t
// Pops a value from the stack.
cvalue_t vm_pop(CurlyVM* vm)
{
	// Error if stack underflow
	if (vm->tos <= vm->stack)
	{
		puts("Error! Stack underflow!");
		vm->running = false;
		return (cvalue_t) 0LL;
	}

	// Pop otherwise
	return *(--vm->tos);
}

// vm_peak(CurlyVM*, size_t) -> cvalue_t
// Peaks in the stack.
cvalue_t vm_peak(CurlyVM* vm, size_t offset)
{
	// Error if stack underflow
	cvalue_t* oos = vm->tos - offset;
	if (oos < vm->stack)
	{
		puts("Error! Stack underflow!");
		vm->running = false;
		return (cvalue_t) 0LL;
	}

	// Peak otherwise
	return *(oos);
}

// clean_vm(CurlyVM*, bool) -> void
// Cleans up a vm.
void clean_vm(CurlyVM* vm, bool clear_globals)
{
	if (vm == NULL)
		return;

	clean_chunk(vm->chunk, clear_globals);
	vm->chunk = NULL;
	vm->pc = NULL;
	free(vm->stack);
	vm->stack = NULL;
	vm->tos = NULL;
	vm->stack_size = 0;

	if (clear_globals)
	{	free(vm->globals);
		vm->globals = NULL;
		vm->globals_count = 0;
		vm->globals_size = 0;
	}
}

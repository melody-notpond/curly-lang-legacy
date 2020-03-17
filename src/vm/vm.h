//
// Curly
// vm.h: Header file for vm.c.
//
// jenra
// March 14 2020
//

#ifndef vm_h
#define vm_h

#include <stdbool.h>

#include "bytecode.h"
#include "types.h"

// Represents a virtual machine for curly.
typedef struct
{
	// The chunk of bytecode being executed.
	chunk_t* chunk;

	// The program counter.
	uint8_t* pc;

	// The stack.
	cvalue_t* stack;

	// The top of the stack.
	cvalue_t* tos;

	// The size of the stack.
	size_t stack_size;

	// The list of all globals.
	cvalue_t* globals;

	// The number of globals.
	size_t globals_count;

	// The size of the globals list.
	size_t globals_size;

	// Whether the vm is running or stopped.
	bool running;
} CurlyVM;

// init_vm(CurlyVM*) -> void
// Initialises a vm.
void init_vm(CurlyVM* vm);

// vm_load(CurlyVM*, chunk_t*) -> void
// Loads a given chunk of bytecode into the vm.
void vm_load(CurlyVM* vm, chunk_t* chunk);

// vm_stepi(CurlyVM*) -> void
// Steps the vm one instruction.
void vm_stepi(CurlyVM* vm);

// vm_run(CurlyVM*) -> void
// Runs the virtual machine.
void vm_run(CurlyVM* vm);

// vm_push(CurlyVM*, cvalue_t) -> void
// Pushes a value onto the stack.
void vm_push(CurlyVM* vm, cvalue_t value);

// vm_pop(CurlyVM*) -> cvalue_t
// Pops a value from the stack.
cvalue_t vm_pop(CurlyVM* vm);

// vm_peak(CurlyVM*, size_t) -> cvalue_t
// Peaks in the stack.
cvalue_t vm_peak(CurlyVM* vm, size_t offset);

// vm_reset(CurlyVM*) -> void
// Resets the vm so it can accept new chunks of bytecode.
void vm_reset(CurlyVM* vm);

// clean_vm(CurlyVM*) -> void
// Cleans up a vm.
void clean_vm(CurlyVM* vm);

#endif /* vm_h */

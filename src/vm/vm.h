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

// Represents a virtual machine for curly.
typedef struct
{
	// The chunk of bytecode being executed.
	chunk_t* chunk;

	// The program counter.
	uint8_t* pc;

	// The stack.
	int64_t* stack;

	// The top of the stack.
	int64_t* tos;

	// The size of the stack.
	size_t stack_size;

	// Whether the vm is running or stopped.
	bool running;
} CurlyVM;

// init_vm(CurlyVM*, chunk_t*) -> void
// Initialises a vm.
void init_vm(CurlyVM* vm, chunk_t* chunk);

// vm_stepi(CurlyVM*) -> void
// Steps the vm one instruction.
void vm_stepi(CurlyVM* vm);

// vm_run(CurlyVM*) -> void
// Runs the virtual machine.
void vm_run(CurlyVM* vm);

// vm_push(CurlyVM*, int64_t) -> void
// Pushes a value onto the stack.
void vm_push(CurlyVM* vm, int64_t value);

// vm_pop(CurlyVM*) -> int64_t
// Pops a value from the stack.
int64_t vm_pop(CurlyVM* vm);

// clean_vm(CurlyVM*) -> void
// Cleans up a vm.
void clean_vm(CurlyVM* vm);

#endif /* vm_h */

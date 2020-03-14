//
// Curly
// vm.h: Header file for vm.c.
//
// jenra
// March 14 2020
//

#ifndef vm_h
#define vm_h

#include "bytecode.h"
#include "opcodes.h"

// Represents a virtual machine for curly.
typedef struct
{
	// The chunk of bytecode being executed.
	chunk_t* chunk;

	// The program counter.
	uint8_t* pc;
} CurlyVM;

// init_vm(CurlyVM*) -> void
// Initialises a vm.
void init_vm(CurlyVM* vm);

// stepi(CurlyVM*) -> void
// Steps the vm one instruction.
void stepi(CurlyVM* vm);

// clean_vm(CurlyVM*) -> void
// Cleans up a vm.
void clean_vm(CurlyVM* vm);

#endif /* vm_h */

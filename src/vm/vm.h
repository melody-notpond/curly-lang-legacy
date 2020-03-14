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

// Represents a virtual machine for curly.
typedef struct
{
	// The chunk of bytecode being executed.
	chunk_t* chunk;

	// The program counter.
	uint8_t* pc;
} CurlyVM;

#endif /* vm_h */

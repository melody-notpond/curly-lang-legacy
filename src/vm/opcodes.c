//
// Curly
// opcodes.c: Implements the opcodes of the virtual machine.
//
// jenra
// March 14 2020
//

#include <stdio.h>

#include "opcodes.h"
#include "types.h"

// The jump table for the opcodes.
int (*opcode_funcs[256])(CurlyVM* vm, uint8_t opcode, uint8_t* pc);

// UNKNOWN
// Implements an unknown opcode.
int opcode_unknown_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	printf("Unknown opcode: 0x%02X (ignored)\n", opcode);
	return 1;
}

// NOP
// Implements no op.
int opcode_nop_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	printf("doing nothing...\n");
	return 1;
}

// BREAK
// Implements break.
int opcode_break_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	printf("HALTING!!\n");
	vm->running = false;
	return 0;
}

// i64 VALUE
// f64 VALUE
// Implements loading values onto the stack.
int opcode_load_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	bool long_op = opcode & 1;

	// Get the index
	int index = *(++pc);
	if (long_op)
	{
		index |= *(++pc) <<  8;
		index |= *(++pc) << 16;
	}

	// Push the constant onto the top of the stack
	vm_push(vm, vm->chunk->pool.values[index]);
	printf("Pushed value #%i onto the stack (%p)\n", index, vm->tos);
	return 2 << long_op;
}

// MUL i64 i64
// MUL i64 f64
// MUL f64 i64
// MUL f64 f64
// Implements multiplication
int opcode_mul_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	// Pop operands from the stack
	cnumb_t b;
	b.i64 = vm_pop(vm);
	cnumb_t a;
	a.i64 = vm_pop(vm);

	// Output inputs
	if (opcode & 2)
		 printf("%f * ",   a.f64);
	else printf("%lli * ", a.i64);
	if (opcode & 1)
		 printf("%f = ",   b.f64);
	else printf("%lli = ", b.i64);

	// Calculate the result
	if (opcode & 2)
		a.f64 *= (opcode & 1 ? b.f64 : b.i64);
	else
		a.i64 *= (opcode & 1 ? b.f64 : b.i64);

	// Output result
	if (opcode & 2)
		 printf("%f\n",   a.f64);
	else printf("%lli\n", a.i64);

	// Push the result onto the stack
	vm_push(vm, a.i64);
	printf("Pushed the result onto the stack (%p)\n", vm->tos);
	return 1;
}

// init_opcodes(void) -> void
// Initialises the jump table.
void init_opcodes()
{
	for (int i = 0; i < 256; i++)
	{
		opcode_funcs[i] = opcode_unknown_func;
	}

	opcode_funcs[OPCODE_NOP			] = opcode_nop_func;
	opcode_funcs[OPCODE_BREAK		] = opcode_break_func;
	opcode_funcs[OPCODE_LOAD		] = opcode_load_func;
	opcode_funcs[OPCODE_LOAD_LONG	] = opcode_load_func;
	opcode_funcs[OPCODE_MUL_I64_I64	] = opcode_mul_func;
	opcode_funcs[OPCODE_MUL_I64_F64	] = opcode_mul_func;
	opcode_funcs[OPCODE_MUL_F64_I64	] = opcode_mul_func;
	opcode_funcs[OPCODE_MUL_F64_F64	] = opcode_mul_func;
}

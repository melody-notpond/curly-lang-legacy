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
	return 2 << long_op;
}

// OPNAME i64 i64
// OPNAME i64 f64
// OPNAME f64 i64
// OPNAME f64 f64
// Implements an infix operation for ints and doubles.
#define infix_op_func(opname, op, preprocessing, postprocessing)				\
int opcode_##opname##_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)			\
{																				\
	/* Pop operands from the stack */											\
	cvalue_t b = vm_pop(vm);													\
	cvalue_t a  = vm_pop(vm);													\
																				\
	/* Do all preprocessing stuff */											\
	preprocessing																\
																				\
	/* Calculate the result */													\
	if (opcode & 3)																\
		a.f64 = (opcode & 2 ? a.f64 : a.i64) op (opcode & 1 ? b.f64 : b.i64);	\
	else																		\
		a.i64 = a.i64 op b.i64;													\
																				\
	/* Do all postprocessing stuff */											\
	postprocessing																\
																				\
	/* Push the result onto the stack */										\
	vm_push(vm, a);																\
	return 1;																	\
}

infix_op_func(mul, *,,)
infix_op_func(div, /,	
	if (b.i64 == 0)
	{
		// Error on divide by zero
		puts("Error: tried to divide by 0.!");
		vm->running = false;
		return 1;
	}
,)
infix_op_func(add, +,,)
infix_op_func(sub, -,,)

#undef infix_func

// MOD i64 i64
// Implements modulo for ints.
// Floating points are unsupported by mod and will never be supported.
int opcode_mod_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	// Pop operands from the stack
	int64_t b = vm_pop(vm).i64;
	int64_t a = vm_pop(vm).i64;

	// Error on divide by zero
	if (b == 0)
	{
		puts("Error: tried to modulo by 0.!");
		vm->running = false;
		return 1;
	}

	// Push result onto the stack
	vm_push(vm, (cvalue_t) (a % b));
	return 1;
}

// POP
// Implements discarding values on the stack.
int opcode_pop_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	vm_pop(vm);
	return 1;
}

// GLOBAL SET VALUE
// Implements creating global variables.
int opcode_set_global_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	// Resize if necessary
	if (vm->globals == NULL)
		vm->globals = calloc((vm->globals_size = 16), 8);
	else if (vm->globals_count >= vm->globals_size)
		vm->globals = realloc(vm->globals, (vm->globals_size <<= 1) << 3);

	// Append the global to the list
	vm->globals[vm->globals_count++] = vm_pop(vm);
	return 1;
}

// GLOBAL VALUE
// Implements pushing global variables onto the stack.
int opcode_global_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
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
	vm_push(vm, *(vm->globals + index));
	return 2 << long_op;
}

// PRINT i64
// PRINT f64
// Implements the temporary print opcode.
int opcode_print_func(CurlyVM* vm, uint8_t opcode, uint8_t* pc)
{
	cvalue_t v = vm_peak(vm, 1);
	if (opcode & 1)
		printf("%f\n",   v.f64);
	else
		printf("%lli\n", v.i64);
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

	opcode_funcs[OPCODE_DIV_I64_I64	] = opcode_div_func;
	opcode_funcs[OPCODE_DIV_I64_F64	] = opcode_div_func;
	opcode_funcs[OPCODE_DIV_F64_I64	] = opcode_div_func;
	opcode_funcs[OPCODE_DIV_F64_F64	] = opcode_div_func;

	opcode_funcs[OPCODE_ADD_I64_I64	] = opcode_add_func;
	opcode_funcs[OPCODE_ADD_I64_F64	] = opcode_add_func;
	opcode_funcs[OPCODE_ADD_F64_I64	] = opcode_add_func;
	opcode_funcs[OPCODE_ADD_F64_F64	] = opcode_add_func;

	opcode_funcs[OPCODE_SUB_I64_I64	] = opcode_sub_func;
	opcode_funcs[OPCODE_SUB_I64_F64	] = opcode_sub_func;
	opcode_funcs[OPCODE_SUB_F64_I64	] = opcode_sub_func;
	opcode_funcs[OPCODE_SUB_F64_F64	] = opcode_sub_func;

	opcode_funcs[OPCODE_MOD			] = opcode_mod_func;

	opcode_funcs[OPCODE_SET_GLOBAL	] = opcode_set_global_func;
	opcode_funcs[OPCODE_GLOBAL		] = opcode_global_func;
	opcode_funcs[OPCODE_GLOBAL_LONG	] = opcode_global_func;
	opcode_funcs[OPCODE_POP			] = opcode_pop_func;

	opcode_funcs[OPCODE_PRINT_I64	] = opcode_print_func;
	opcode_funcs[OPCODE_PRINT_F64	] = opcode_print_func;
}

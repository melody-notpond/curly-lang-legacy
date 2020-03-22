//
// Curly
// ir.h: Header file for ir.c.
//
// jenra
// March 21 2020
//

#ifndef curly_ir_h
#define curly_ir_h

#include <inttypes.h>
#include <stdbool.h>

// Represents an argument to a line of ir code.
typedef struct
{
	// The type of the argument.
	int8_t type;

	// The value of the argument.
	char* value;
} ir_arg_t;

// Represents a line of ir code.
typedef struct
{
	// The operator of the line.
	char* op;

	// The arguments of the line.
	ir_arg_t* args;
} ir_line_t;

// Represents a basic block of ir code.
// A basic block is a block of code in which there are no jumps.
typedef struct s_ir_block
{
	// The lines of code in the basic block.
	ir_line_t* lines;

	// The type of the jump.
	int8_t jump_type;

	// The block that this block jumps to.
	struct s_ir_block* cond_jump;
} ir_block_t;

// Represents a list of blocks
typedef struct
{
	// The list of blocks.
	ir_block_t* blocks;
} curly_ir_t;

#endif /* curly_ir_h */

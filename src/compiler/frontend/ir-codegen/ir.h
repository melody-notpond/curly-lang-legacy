//
// Curly
// ir.h: Header file for ir.c.
//
// jenra
// March 21 2020
//

#ifndef curly_ir_h
#define curly_ir_h

#include "../correctness/scopes.h"
#include <stdbool.h>

// Represents an argument to a line of ir code.
typedef struct
{
	// The type of the argument.
	curly_type_t type;

	// The value of the argument.
	char* value;
} ir_arg_t;

// Represents a line of ir code.
typedef struct
{
	// The operator of the line.
	char* op;

	// The arguments of the line.
	ir_arg_t args[2];

	// The output type of the instruction.
	curly_type_t type;
} ir_line_t;

typedef enum
{
	BLOCK_JUMP_TYPE_ALWAYS,
	BLOCK_JUMP_TYPE_TRUE,
	BLOCK_JUMP_TYPE_FALSE
} block_jump_type_t;

// Represents a basic block of ir code.
// A basic block is a block of code in which there are no jumps.
typedef struct s_ir_block
{
	// The lines of code in the basic block.
	ir_line_t* lines;

	// The number of lines.
	int count;

	// The amount of memory allocated for the lines.
	int size;

	// The type of the jump.
	block_jump_type_t jump_type;

	// The block that this block jumps to.
	struct s_ir_block* cond_jump;
} ir_block_t;

// Represents a list of blocks
typedef struct
{
	// The list of blocks.
	ir_block_t* blocks;

	// The number of blocks.
	int count;

	// The amount of memory allocated for the blocks.
	int size;
} curly_ir_t;

// init_ir_arg(void) -> ir_arg_t
// Initialises an empty ir arg.
ir_arg_t init_ir_arg();

// init_ir_line(void) -> ir_line_t
// Initialises an empty ir line.
ir_line_t init_ir_line();

// init_ir_block(void) -> ir_block_t
// Initialises an empty ir block.
ir_block_t init_ir_block();

// init_ir(void) -> curly_ir_t
// Initialises an empty ir.
curly_ir_t init_ir();

// add_line(curly_ir_t*, ir_line_t*) -> void
// Adds a new line to the current block.
void add_line(curly_ir_t* ir, ir_line_t* line);

// print_ir(curly_ir_t*) -> void
// Prints ir code to stdout.
void print_ir(curly_ir_t* ir);

// clean_ir(curly_ir_t*) -> void
// Cleans an ir.
void clean_ir(curly_ir_t* ir);

// clean_ir_block(ir_block_t*) -> void
// Cleans an ir block.
void clean_ir_block(ir_block_t* block);

// clean_ir_line(ir_line_t*) -> void
// Cleans an ir line.
void clean_ir_line(ir_line_t* line);

// clean_ir_arg(ir_arg_t*) -> void
// Cleans an ir arg.
void clean_ir_arg(ir_arg_t* arg);

#endif /* curly_ir_h */

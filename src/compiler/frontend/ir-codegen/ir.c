//
// Curly
// ir.c: Implements the data structure that holds the ir code.
//
// jenra
// March 21 2020
//

#include <stdio.h>
#include <stdlib.h>

#include "ir.h"

// init_ir_arg(void) -> ir_arg_t
// Initialises an empty ir arg.
ir_arg_t init_ir_arg()
{
	ir_arg_t arg;
	arg.type = 0;
	arg.value = NULL;
	return arg;
}

// init_ir_line(void) -> ir_line_t
// Initialises an empty ir line.
ir_line_t init_ir_line()
{
	ir_line_t line;
	line.op = NULL;
	line.args[0] = init_ir_arg();
	line.args[1] = init_ir_arg();
	return line;
}

// init_ir_block(void) -> ir_block_t
// Initialises an empty ir block.
ir_block_t init_ir_block()
{
	ir_block_t block;
	block.lines = NULL;
	block.count = 0;
	block.size = 0;
	block.jump_type = 0;
	block.cond_jump = NULL;
	return block;
}

// init_ir(void) -> curly_ir_t
// Initialises an empty ir.
curly_ir_t init_ir()
{
	curly_ir_t ir;
	ir.blocks = malloc(sizeof(ir_block_t));
	*ir.blocks = init_ir_block();
	ir.count = 1;
	ir.size = 1;
	return ir;
}

// append_element(void*, size_t&, size_t&, type, type_t) -> void
// Appends an element to a list.
#define append_element(values, count, size, type, value) do				\
{																		\
	/* Resize the list if necessary */									\
	if (size == 0)														\
		values = calloc((size = 8), sizeof(type));						\
	else if (count >= size)												\
		values = realloc(values, (size <<= 1) * sizeof(type));			\
																		\
	/* Append the value */												\
	values[count++] = value;											\
} while (0)

// add_line(curly_ir_t*, ir_line_t*) -> void
// Adds a new line to the current block.
void add_line(curly_ir_t* ir, ir_line_t* line)
{
	ir_block_t* block = ir->blocks + ir->count - 1;
	append_element(block->lines, block->count, block->size, ir_line_t, *line);
}

#undef append_element

// print_ir(curly_ir_t*) -> void
// Prints ir code to stdout.
void print_ir(curly_ir_t* ir)
{
	for (int i = 0; i < ir->count; i++)
	{
		ir_block_t* block = ir->blocks + i;
		for (int j = 0; j < block->count; j++)
		{
			// Print out the line
			ir_line_t* line = block->lines + j;
			printf("%s", line->op);

			// Print out any arguments
			if (line->args[0].value != NULL)
				printf(" %s", line->args[0].value);
			if (line->args[1].value != NULL)
				printf(" %s", line->args[1].value);
			puts("");
		}
	}
}

// clean_ir(curly_ir_t*) -> void
// Cleans an ir.
void clean_ir(curly_ir_t* ir)
{
	for (int i = 0; i < ir->count; i++)
	{
		clean_ir_block(ir->blocks + i);
	}
}

// clean_ir_block(ir_block_t*) -> void
// Cleans an ir block.
void clean_ir_block(ir_block_t* block)
{
	for (int i = 0; i < block->count; i++)
	{
		clean_ir_line(block->lines + i);
	}
}

// clean_ir_line(ir_line_t*) -> void
// Cleans an ir line.
void clean_ir_line(ir_line_t* line)
{
	free(line->op);
	clean_ir_arg(line->args    );
	clean_ir_arg(line->args + 1);
}

// clean_ir_arg(ir_arg_t*) -> void
// Cleans an ir arg.
void clean_ir_arg(ir_arg_t* arg)
{
	free(arg->value);
}

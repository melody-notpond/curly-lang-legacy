//
// Curly
// compile_bytecode.c: Compiles the language into the virtual machine's bytecode.
//
// jenra
// March 15 2020
//

#include "compile_bytecode.h"
#include "../../../vm/opcodes.h"

// Represents the result from compiling a subtree.
// This is used to determine the type for the print opcode.
typedef enum
{
	COMPILE_RESULT_ERROR = 0,
	COMPILE_RESULT_INT,
	COMPILE_RESULT_FLOAT
} compile_result_t;

compile_result_t infix_chunk(chunk_t* chunk, ast_t* tree);
compile_result_t tree_chunk(chunk_t* chunk, ast_t* tree);

// infix_chunk(chunk_t*, ast_t*) -> compile_result_t
// Compiles an infix subtree into bytecode.
compile_result_t infix_chunk(chunk_t* chunk, ast_t* tree)
{
	compile_result_t left = COMPILE_RESULT_INT;
	for (int i = 0; i < tree->children_count; i += 2)
	{
		// Add operands to the chunk
		compile_result_t right = tree_chunk(chunk, tree->children + i);

		// Check return type
		if (!right) return right;
		
		if (i != 0)
		{
			// Get the operator
			ast_t* op = tree->children + i - 1;

			// The node should be an operator
			if (strcmp(op->name, "op"))
				return COMPILE_RESULT_ERROR;

			// Write the appropriate opcode
			uint8_t option = (left - COMPILE_RESULT_INT) << 1 | (right - COMPILE_RESULT_INT);
			if (!strcmp(op->value, "*"))
				write_chunk(chunk, OPCODE_MUL_I64_I64 | option);
			else if (!strcmp(op->value, "/"))
				write_chunk(chunk, OPCODE_DIV_I64_I64 | option);
			else if (!strcmp(op->value, "+"))
				write_chunk(chunk, OPCODE_ADD_I64_I64 | option);
			else if (!strcmp(op->value, "-"))
				write_chunk(chunk, OPCODE_SUB_I64_I64 | option);
			else if (!strcmp(op->value, "%"))
			{
				// Modulo only takes in integers
				if (option)
				{
					puts("Cannot modulo doubles!");
					return COMPILE_RESULT_ERROR;
				}
				write_chunk(chunk, OPCODE_MOD);
			}
		}

		// Update the left side
		if (right == COMPILE_RESULT_FLOAT)
			left = right;
	}
	return left;
}

// tree_chunk(chunk_t*, ast_t*) -> compile_result_t
// Compiles a subtree into bytecode.
compile_result_t tree_chunk(chunk_t* chunk, ast_t* tree)
{
	char* name = tree->name;
	if (!strcmp(name, "int"))
	{
		// Add an integer load instruction
		chunk_add_i64(chunk, atoll(tree->value));
		return COMPILE_RESULT_INT;
	} else if (!strcmp(name, "float"))
	{
		// Add a double load instruction
		chunk_add_f64(chunk, atof (tree->value));
		return COMPILE_RESULT_FLOAT;
	} else if (!strcmp(name, "infix"))
		// Compile the infix subtree
		return infix_chunk(chunk, tree);
	else
	{
		// Invalid form
		printf("error");
		return COMPILE_RESULT_ERROR;
	}
}

// compile_tree(chunk_t*, parse_result_t*, bool) -> bool
// Compiles the ast into a chunk of bytecode.
bool compile_tree(chunk_t* chunk, parse_result_t* result, bool terminate)
{
	// Only compile successfully parsed trees
	if (!result->succ)
		return false;

	ast_t root = result->ast;
	if (!strcmp(root.name, "root"))
	{
		for (int i = 0; i < root.children_count; i++)
		{
			// Compile every subtree of the root node
			compile_result_t res = tree_chunk(chunk, root.children + i);
			if (!res) return false;

			// Determine type for printing
			if (res == COMPILE_RESULT_INT)
				write_chunk(chunk, OPCODE_PRINT_I64);
			else if (res == COMPILE_RESULT_FLOAT)
				write_chunk(chunk, OPCODE_PRINT_F64);
		}

		// Optionally add a terminating break instruction
		if (terminate)
			write_chunk(chunk, OPCODE_BREAK);
		return true;
	} else
	{
		// Compile the root node
		compile_result_t res = tree_chunk(chunk, &root);
		if (!res) return false;

		// Determine the type for printing
		if (res == COMPILE_RESULT_INT)
			write_chunk(chunk, OPCODE_PRINT_I64);
		else if (res == COMPILE_RESULT_FLOAT)
			write_chunk(chunk, OPCODE_PRINT_F64);

		// Optionally add a terminating break instruction
		if (terminate)
			write_chunk(chunk, OPCODE_BREAK);
		return true;
	}
}

//
// Curly
// compile_bytecode.c: Compiles the language into the virtual machine's bytecode.
//
// jenra
// March 15 2020
//

#include "compile_bytecode.h"
#include "../../../vm/opcodes.h"

bool infix_chunk(chunk_t* chunk, ast_t* tree);
bool tree_chunk(chunk_t* chunk, ast_t* tree);

// infix_chunk(chunk_t*, ast_t*) -> bool
// Compiles an infix subtree into bytecode.
bool infix_chunk(chunk_t* chunk, ast_t* tree)
{
	for (int i = 0; i < tree->children_count; i += 2)
	{
		// Add operands to the chunk
		if (!tree_chunk(chunk, tree->children + i))
			return false;
	}
	return true;
}

// tree_chunk(chunk_t*, ast_t*) -> bool
// Compiles a subtree into bytecode.
bool tree_chunk(chunk_t* chunk, ast_t* tree)
{
	char* name = tree->name;
	if (!strcmp(name, "int"))
		// Add an integer load instruction
		chunk_add_i64(chunk, atoll(tree->value));
	else if (!strcmp(name, "float"))
		// Add a double load instruction
		chunk_add_f64(chunk, atof (tree->value));
	else if (!strcmp(name, "infix"))
		// Compile the infix subtree
		return infix_chunk(chunk, tree);
	else
	{
		// Invalid form
		printf("error");
		return false;
	}
	return true;
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
		// Compile every subtree of the root node
		for (int i = 0; i < root.children_count; i++)
		{
			if (!tree_chunk(chunk, root.children + i))
				return false;
		}

		// Optionally add a terminating break instruction
		if (terminate)
			write_chunk(chunk, OPCODE_BREAK);
		return true;
	} else
	{
		// Compile the root node
		if (!tree_chunk(chunk, &root))
			return false;

		// Optionally add a terminating break instruction
		if (terminate)
			write_chunk(chunk, OPCODE_BREAK);
		return true;
	}
}

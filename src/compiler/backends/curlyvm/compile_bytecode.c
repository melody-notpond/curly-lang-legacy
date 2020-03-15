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

bool infix_chunk(chunk_t* chunk, ast_t* tree)
{
	for (int i = 0; i < tree->children_count; i += 2)
	{
		if (!tree_chunk(chunk, tree->children + i))
			return false;
	}
	return true;
}

bool tree_chunk(chunk_t* chunk, ast_t* tree)
{
	char* name = tree->name;
	if (!strcmp(name, "int"))
		chunk_add_i64(chunk, atoll(tree->value));
	else if (!strcmp(name, "float"))
		chunk_add_f64(chunk, atof (tree->value));
	else if (!strcmp(name, "infix"))
		return infix_chunk(chunk, tree);
	else
	{
		printf("error");
		return false;
	}
	return true;
}

// compile_tree(chunk_t*, parse_result_t*, bool) -> bool
// Compiles the ast into a chunk of bytecode.
bool compile_tree(chunk_t* chunk, parse_result_t* result, bool terminate)
{
	if (!result->succ)
		return false;

	ast_t root = result->ast;
	if (!strcmp(root.name, "root"))
	{
		for (int i = 0; i < root.children_count; i++)
		{
			if (!tree_chunk(chunk, root.children + i))
				return false;
		}
		if (terminate)
			write_chunk(chunk, OPCODE_BREAK);
		return true;
	} else
	{
		if (!tree_chunk(chunk, &root))
			return false;
		if (terminate)
			write_chunk(chunk, OPCODE_BREAK);
		return true;
	}
}

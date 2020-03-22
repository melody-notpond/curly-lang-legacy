//
// Curly
// gencode.c: Generates ir code from an ast and checks correctness.
//
// jenra
// March 21 2020
//

#include "gencode.h"

// tree_ir(compiler_t*, ast_t*) -> void
// Converts a subtree into ir.
void tree_ir(compiler_t* state, ast_t* tree)
{
	char* name = tree->name;
	ir_line_t line = init_ir_line();

	if (!strcmp(name, "int"))
	{
		// Integer constant
		line.op = strdup("load");
		line.args[0].value = strdup(tree->value);
		line.type = CURLY_TYPE_INT;
	} else if (!strcmp(name, "float"))
	{
		// Floating point constant
		line.op = strdup("load");
		line.args[0].value = strdup(tree->value);
		line.type = CURLY_TYPE_FLOAT;
	} else
	{
		// Invalid form
		puts("Error: Invalid syntax tree passed");
		state->cause = tree;
		state->type_cause = CURLY_TYPE_DNE;
		state->status = -1;
		return;
	}

	add_line(&state->ir, &line);
}

// convert_tree_ir(compiler_t*, parse_result_t*) -> void
// Converts the ast into ir.
void convert_tree_ir(compiler_t* state, parse_result_t* result)
{
	// Only compile successfully parsed trees
	init_compiler_state(state);
	if (!result->succ)
		return;

	ast_t root = result->ast;

	if (!strcmp(root.name, "root"))
	{
		for (int i = 0; i < root.children_count; i++)
		{
			// Compile every subtree of the root node
			tree_ir(state, root.children + i);

			// Error result
			if (state->cause)
			{
				clean_ir(&state->ir);
				return;
			}
		}
	} else
	{
		// Compile the root node
		tree_ir(state, &root);

		// Error result
		if (state->cause)
		{
			clean_ir(&state->ir);
			return;
		}
	}
}

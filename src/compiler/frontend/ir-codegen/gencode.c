//
// Curly
// gencode.c: Generates ir code from an ast and checks correctness.
//
// jenra
// March 21 2020
//

#include "../correctness/compiler_struct.h"
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
		line.op = strdup("i64");
		line.args[0].type = SCOPE_CURLY_TYPE_INT;
		line.args[0].value = strdup(tree->value);
	} else if (!strcmp(name, "float"))
	{
		// Floating point constant
		line.op = strdup("f64");
		line.args[0].type = SCOPE_CURLY_TYPE_INT;
		line.args[0].value = strdup(tree->value);
	} else
	{
		// Invalid form
		puts("Error: Invalid syntax tree passed");
		state->cause = tree;
		state->type_cause = SCOPE_CURLY_TYPE_DNE;
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
	if (!result->succ)
		return;

	ast_t root = result->ast;
	if (!strcmp(root.name, "root"))
	{
		for (int i = 0; i < root.children_count; i++)
		{
			// Compile every subtree of the root node
			tree_ir(state, root.children + i);
			if (state->cause) return;
		}
	} else
	{
		// Compile the root node
		tree_ir(state, &root);
		if (state->cause) return;
	}
}

//
// Curly
// gencode.c: Generates ir code from an ast and checks correctness.
//
// jenra
// March 21 2020
//

#include "gencode.h"

curly_type_t infix_ir(compiler_t* state, ast_t* tree);
curly_type_t tree_ir (compiler_t* state, ast_t* tree);

// infix_ir(compiler_t*, ast_t*) -> curly_type_t
// Converts an infix subtree into ir.
curly_type_t infix_ir(compiler_t* state, ast_t* tree)
{
	curly_type_t left = CURLY_TYPE_INT;
	for (int i = 0; i < tree->children_count; i += 2)
	{
		// Add operands to the ir
		curly_type_t right = tree_ir(state, tree->children + i);
		if (state->cause) return CURLY_TYPE_DNE;

		// Check return type
		if (right != CURLY_TYPE_INT && right != CURLY_TYPE_FLOAT)
		{
			state->cause = tree->children + i;
			state->type_cause = right;
			state->status = -1;
			return CURLY_TYPE_DNE;
		}

		if (i != 0)
		{
			// Get the operator
			ast_t* op = tree->children + i - 1;

			// Set up the line of ir
			ir_line_t line = init_ir_line();
			line.args[0].type = left;
			line.args[1].type = right;
			line.type = (right == CURLY_TYPE_FLOAT ? right : left);

			// Write the operator
			if (!strcmp(op->value, "*"))
				line.op = strdup("mul");
			else if (!strcmp(op->value, "/"))
				line.op = strdup("div");
			else if (!strcmp(op->value, "+"))
				line.op = strdup("add");
			else if (!strcmp(op->value, "-"))
				line.op = strdup("sub");
			else if (!strcmp(op->value, "%"))
			{
				// Modulo only takes in integers
				if (line.type != CURLY_TYPE_INT)
				{
					state->cause = op + (left == CURLY_TYPE_FLOAT ? -1 : 1);
					state->type_cause = CURLY_TYPE_FLOAT;
					state->status = -1;
					return CURLY_TYPE_DNE;
				} else line.op = strdup("mod");
			}

			add_line(&state->ir, &line);
		}

		// Update the left side
		if (right == CURLY_TYPE_FLOAT)
			left = right;
	}
	return left;
}

// tree_ir(compiler_t*, ast_t*) -> curly_type_t
// Converts a subtree into ir.
curly_type_t tree_ir(compiler_t* state, ast_t* tree)
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
	} else if (!strcmp(name, "infix"))
		// Infix expression
		return infix_ir(state, tree);
	else
	{
		state->cause = tree;
		state->type_cause = CURLY_TYPE_DNE;
		state->status = -1;
		return CURLY_TYPE_DNE;
	}

	add_line(&state->ir, &line);
	return line.type;
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

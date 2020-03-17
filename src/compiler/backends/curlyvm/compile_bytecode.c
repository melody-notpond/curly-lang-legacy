//
// Curly
// compile_bytecode.c: Compiles the language into the virtual machine's bytecode.
//
// jenra
// March 15 2020
//

#include "compile_bytecode.h"
#include "../../../vm/opcodes.h"

curly_type_t infix_chunk(vm_compiler_t* state, ast_t* tree);
curly_type_t tree_chunk(vm_compiler_t* state, ast_t* tree);

// infix_chunk(vm_compiler_t*, ast_t*) -> curly_type_t
// Compiles an infix subtree into bytecode.
curly_type_t infix_chunk(vm_compiler_t* state, ast_t* tree)
{
	curly_type_t left = SCOPE_CURLY_TYPE_INT;
	for (int i = 0; i < tree->children_count; i += 2)
	{
		// Add operands to the chunk
		curly_type_t right = tree_chunk(state, tree->children + i);
		if (state->state.cause) return SCOPE_CURLY_TYPE_DNE;

		// Check return type
		if (right != SCOPE_CURLY_TYPE_INT && right != SCOPE_CURLY_TYPE_FLOAT)
		{
			state->state.cause = tree->children + i;
			state->state.type_cause = right;
			state->state.status = -1;
			return SCOPE_CURLY_TYPE_DNE;
		}

		if (i != 0)
		{
			// Get the operator
			ast_t* op = tree->children + i - 1;

			// Write the appropriate opcode
			uint8_t option = (left == SCOPE_CURLY_TYPE_FLOAT) << 1 | (right == SCOPE_CURLY_TYPE_FLOAT);
			if (!strcmp(op->value, "*"))
				write_chunk(state->chunk, OPCODE_MUL_I64_I64 | option);
			else if (!strcmp(op->value, "/"))
				write_chunk(state->chunk, OPCODE_DIV_I64_I64 | option);
			else if (!strcmp(op->value, "+"))
				write_chunk(state->chunk, OPCODE_ADD_I64_I64 | option);
			else if (!strcmp(op->value, "-"))
				write_chunk(state->chunk, OPCODE_SUB_I64_I64 | option);
			else if (!strcmp(op->value, "%"))
			{
				// Modulo only takes in integers
				if (option)
				{
					state->state.cause = op + (left == SCOPE_CURLY_TYPE_FLOAT ? -1 : 1);
					state->state.type_cause = SCOPE_CURLY_TYPE_FLOAT;
					state->state.status = -1;
					return SCOPE_CURLY_TYPE_DNE;
				}
				write_chunk(state->chunk, OPCODE_MOD);
			}
		}

		// Update the left side
		if (right == SCOPE_CURLY_TYPE_FLOAT)
			left = right;
	}
	return left;
}

// assign_chunk(vm_compiler_t*, ast_t*) -> curly_type_t
// Compiles an assignment subtree into bytecode.
curly_type_t assign_chunk(vm_compiler_t* state, ast_t* tree)
{
	// Search for the global
	ast_t* symbol = tree->children + 0;
	int index = search_global(&state->state.scope, symbol->value);

	// If the global exists, error
	if (index != -1)
	{
		state->state.cause = symbol;
		state->state.type_cause = SCOPE_CURLY_TYPE_FLOAT;
		state->state.status = -1;
		return SCOPE_CURLY_TYPE_DNE;
	}

	// Find the type
	curly_type_t type = tree_chunk(state, tree->children + 1);
	if (state->state.cause) return SCOPE_CURLY_TYPE_DNE;

	// Create the global
	chunk_global(state->chunk, symbol->value);
	add_variable(&state->state.scope, symbol->value, type);
	return SCOPE_CURLY_TYPE_DNE;
}

// tree_chunk(vm_compiler_t*, ast_t*) -> curly_type_t
// Compiles a subtree into bytecode.
curly_type_t tree_chunk(vm_compiler_t* state, ast_t* tree)
{
	char* name = tree->name;
	if (!strcmp(name, "symbol"))
	{
		// Find the variable (only globals for now)
		char* var = tree->value;
		int index = search_global(&state->state.scope, var);

		if (index != -1)
		{
			// Success!
			chunk_global(state->chunk, var);
			return state->state.scope.global.types[index];
		} else
		{
			// The variable doesn't exist
			state->state.cause = tree;
			state->state.status = -1;
			state->state.type_cause = SCOPE_CURLY_TYPE_DNE;
			return SCOPE_CURLY_TYPE_DNE;
		}
	} else if (!strcmp(name, "int"))
	{
		// Add an integer load instruction
		chunk_add_i64(state->chunk, atoll(tree->value));
		return SCOPE_CURLY_TYPE_INT;
	} else if (!strcmp(name, "float"))
	{
		// Add a double load instruction
		chunk_add_f64(state->chunk, atof (tree->value));
		return SCOPE_CURLY_TYPE_FLOAT;
	} else if (!strcmp(name, "string"))
	{
		// Allocate a copy
		char* str = tree->value + 1;
		char* copy = malloc(strlen(str));
		char* cp_i = copy;

		// Copy the string
		for (; *(str + 1); str++, cp_i++)
		{
			if (*str == '\\')
			{
				// Convert escape sequences
				switch (*(++str))
				{
					case 'n':
						*cp_i = '\n';
						break;
					case 't':
						*cp_i = '\t';
						break;
					case '"':
						*cp_i = '\"';
						break;
					case '\'':
						*cp_i = '\'';
						break;
					case '\\':
						*cp_i = '\\';
						break;
					default:
						// By default, just copy the characters over
						// This is useful for regexes
						*cp_i++ = '\\';
						*cp_i = *str;
						break;
				}

			// Copy the character
			} else *cp_i = *str;
		}

		// Add the null character and add the string
		*cp_i = '\0';
		chunk_add_string(state->chunk, copy);
		free(copy);
		return SCOPE_CURLY_TYPE_STRING;
	} else if (!strcmp(name, "infix"))
		// Compile the infix subtree
		return infix_chunk(state, tree);
	else if (!strcmp(name, "assign"))
		return assign_chunk(state, tree);
	else
	{
		// Invalid form
		puts("Error: Invalid syntax tree passed");
		state->state.cause = tree;
		state->state.type_cause = SCOPE_CURLY_TYPE_DNE;
		state->state.status = -1;
		return SCOPE_CURLY_TYPE_DNE;
	}
}

// compile_tree(vm_compiler_t*, parse_result_t*, bool) -> void
// Compiles the ast into a chunk of bytecode.
void compile_tree(vm_compiler_t* state, parse_result_t* result, bool terminate)
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
			curly_type_t res = tree_chunk(state, root.children + i);
			if (state->state.cause) return;

			// Determine type for printing
			if (res == SCOPE_CURLY_TYPE_INT)
				write_chunk(state->chunk, OPCODE_PRINT_I64);
			else if (res == SCOPE_CURLY_TYPE_FLOAT)
				write_chunk(state->chunk, OPCODE_PRINT_F64);
			else if (res == SCOPE_CURLY_TYPE_STRING)
				write_chunk(state->chunk, OPCODE_PRINT_STR);
		}

		// Optionally add a terminating break instruction
		if (terminate)
			write_chunk(state->chunk, OPCODE_BREAK);
	} else
	{
		// Compile the root node
		curly_type_t res = tree_chunk(state, &root);
		if (state->state.cause) return;

		// Determine the type for printing
		if (res == SCOPE_CURLY_TYPE_INT)
			write_chunk(state->chunk, OPCODE_PRINT_I64);
		else if (res == SCOPE_CURLY_TYPE_FLOAT)
			write_chunk(state->chunk, OPCODE_PRINT_F64);
		else if (res == SCOPE_CURLY_TYPE_STRING)
			write_chunk(state->chunk, OPCODE_PRINT_STR);

		// Optionally add a terminating break instruction
		if (terminate)
			write_chunk(state->chunk, OPCODE_BREAK);
	}
}

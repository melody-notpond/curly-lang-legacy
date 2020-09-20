// 
// correctness
// scope.c: Implements a scope system.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#include "scope.h"

// push_scope(ir_scope_t*) -> ir_scope_t*
// Initialises a scope and pushes it onto the stack of scopes.
ir_scope_t* push_scope(ir_scope_t* parent)
{
	ir_scope_t* scope = malloc(sizeof(ir_scope_t));
	scope->var_types = init_hashmap();
	scope->var_vals = init_hashmap();
	scope->types = init_hashmap();

	for (int i = 0; i < INFIX_OP_COUNT; i++)
	{
		scope->infix_ops[i] = NULL;
	}

	scope->parent = parent;
	return scope;
}

// convert_infix_op(char*) -> int
// Converts an infix operator into its coresponding index in the list of type definitions.
int convert_infix_op(char* op)
{
	if (op == NULL)
		return -1;

	switch (op[0])
	{
		case '*':
			return 1;
		case '/':
			return 2;
		case '%':
			return 3;
		case '+':
			return 4;
		case '-':
			return 5;
		case '<':
			return op[1] == '<' && op[2] == '\0' ? 6 : -1;
		case '>':
			return op[1] == '>' && op[2] == '\0' ? 7 : -1;
		case '&':
			return 8;
		case '|':
			return 9;
		case '^':
			return 10;
		default:
			return -1;
	}
}

// add_prefix_op(ir_scope_t*, type_t*, type_t*) -> void
// Adds a new prefix operator to the scope.
void add_prefix_op(ir_scope_t* scope, type_t* operand, type_t* out)
{
	// Create the type
	type_t* type = init_type(IR_TYPES_FUNC, NULL, 2);
	type->field_types[0] = operand;
	type->field_types[1] = out;

	// Create the prefix operator
	ir_infix_type_t* prefix = malloc(sizeof(ir_infix_type_t));
	prefix->type = type;
	prefix->next = NULL;

	// Append the new operator
	if (scope->infix_ops[0] == NULL)
		scope->infix_ops[0] = prefix;
	else
	{
		// Find the last element in the linked list
		ir_infix_type_t* head = scope->infix_ops[0];
		while (head->next != NULL)
		{
			head = head->next;
		}
		head->next = prefix;
	}
}

// add_infix_op(ir_scope_t*, char*, type_t*, type_t*, type_t*) -> void
// Adds an infix operation to the scope.
void add_infix_op(ir_scope_t* scope, char* op, type_t* left, type_t* right, type_t* out)
{
	// Check that the op is valid
	int op_index = convert_infix_op(op);
	if (op_index < 0) return;

	// Create the type
	type_t* type = init_type(IR_TYPES_FUNC, NULL, 3);
	type->field_types[0] = left;
	type->field_types[1] = right;
	type->field_types[2] = out;

	// Create the infix operator
	ir_infix_type_t* infix = malloc(sizeof(ir_infix_type_t));
	infix->type = type;
	infix->next = NULL;

	// Append the new operator
	if (scope->infix_ops[op_index] == NULL)
		scope->infix_ops[op_index] = infix;
	else
	{
		// Find the last element in the linked list
		ir_infix_type_t* head = scope->infix_ops[op_index];
		while (head->next != NULL)
		{
			head = head->next;
		}
		head->next = infix;
	}
}

// scope_lookup_prefix(ir_scope_t*, type_t*) -> type_t*
// Looks up a prefix operator based on the argument type and returns the result type.
type_t* scope_lookup_prefix(ir_scope_t* scope, type_t* operand)
{
	// Iterate over the scopes
	ir_scope_t* top = scope;
	while (top != NULL)
	{
		// Search for the prefix operator
		ir_infix_type_t* current = top->infix_ops[0];
		while (current != NULL)
		{
			if (type_subtype(current->type->field_types[0], operand, false))
				return current->type->field_types[1];
			current = current->next;
		}

		// Get parent scope
		top = top->parent;
	}

	// No operator found
	return NULL;
}

// scope_lookup_infix(ir_scope_t*, char*, type_t*, type_t*) -> type_t*
// Looks up an infix operator based on the argument types and returns the result type.
type_t* scope_lookup_infix(ir_scope_t* scope, char* op, type_t* left, type_t* right)
{
	// Check that the op is valid
	int op_index = convert_infix_op(op);
	if (op_index < 0) return NULL;

	// Iterate over the scopes
	ir_scope_t* top = scope;
	while (top != NULL)
	{
		// Search for the infix operator in topmost scope
		ir_infix_type_t* current = top->infix_ops[op_index];
		while (current != NULL)
		{
			if (type_subtype(current->type->field_types[0], left, false) && type_subtype(current->type->field_types[1], right, false))
				return current->type->field_types[2];
			current = current->next;
		}

		// Get parent scope
		top = top->parent;
	}

	// No operator found
	return NULL;
}

// scope_lookup_var_type(ir_scope_t*, char*) -> type_t*
// Looks up the type of a variable in the scope.
type_t* scope_lookup_var_type(ir_scope_t* scope, char* name)
{
	// Iterate over every scope
	while (scope != NULL)
	{
		// Get the type in the current scope
		type_t* type = map_get(scope->var_types, name);

		// Return the type if found
		if (type != NULL)
			return type;

		// Get parent scope
		scope = scope->parent;
	}

	// No type was found
	return NULL;
}

// scope_lookup_var_val(ir_scope_t*, char*) -> ast_t*
// Looks up the value of a variable in the scope.
ast_t* scope_lookup_var_val(ir_scope_t* scope, char* name)
{
	// Iterate over every scope
	while (scope != NULL)
	{
		// Get the ast in the current scope
		ast_t* ast = map_get(scope->var_vals, name);

		// Return the ast if found
		if (ast != NULL)
			return ast;

		// Get parent scope
		scope = scope->parent;
	}

	// No ast was found
	return NULL;
}

// scope_lookup_type(ir_scope_t*, char*) -> type_t*
// Looks up a type in the scope.
type_t* scope_lookup_type(ir_scope_t* scope, char* name)
{
	// Iterate over every scope
	while (scope != NULL)
	{
		// Get the type in the current scope
		type_t* type = map_get(scope->types, name);

		// Return the type if found
		if (type != NULL)
			return type;

		// Get parent scope
		scope = scope->parent;
	}

	// No type was found
	return NULL;
}

// pop_scope(ir_scope_t* scope)
// Deletes a scope and returns its parent.
ir_scope_t* pop_scope(ir_scope_t* scope)
{
	del_hashmap(scope->var_types);
	del_hashmap(scope->var_vals);
	del_hashmap(scope->types);

	for (int i = 0; i < INFIX_OP_COUNT; i++)
	{
		ir_infix_type_t* head = scope->infix_ops[i];
		while (head != NULL)
		{
			ir_infix_type_t* next = head->next;
			free(head);
			head = next;
		}
	}

	ir_scope_t* parent = scope->parent;
	free(scope);
	return parent;
}

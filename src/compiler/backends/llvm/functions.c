// 
// llvm
// functions.c: Implements helper functions for creating functions.
// 
// Created by jenra.
// Created on October 2 2020.
// 

#include <string.h>

#include "functions.h"

// find_llvm_closure_locals(llvm_codegen_env_t*, ast_t*, hashmap_t*) -> void
// Finds all locals the function closes over and adds them to the hash of closed locals.
void find_llvm_closure_locals(llvm_codegen_env_t* env, ast_t* body, hashmap_t* closed_locals)
{
	if (body->value.tag == LEX_TAG_OPERAND)
	{
		// Local is closed from above scope
		LLVMValueRef value = lookup_llvm_local(env, body->value.value);
		if (body->value.type == LEX_TYPE_SYMBOL && value != NULL)
			map_add(closed_locals, body->value.value, value);
	} else if (!strcmp(body->value.value, "with"))
	{
		// Create a new scope
		env->local = push_llvm_scope(env->local);

		// Push variables
		for (size_t i = 0; i < body->children_count - 1; i++)
		{
			// Get name
			char* name = NULL;
			ast_t* ast = body->children[i];
			if (ast->children[0]->value.type == LEX_TYPE_SYMBOL)
				name = ast->children[0]->value.value;
			else if (ast->children[0]->value.type == LEX_TYPE_COLON)
				name = ast->children[0]->children[0]->value.value;

			// Set local
			set_llvm_local(env, name, NULL);

			// Set arguments if applicable
			if (ast->children[0]->value.type == LEX_TYPE_SYMBOL && ast->children[0]->children_count != 0)
			{
				for (size_t i = 0; i < ast->children[0]->children_count; i++)
				{
					ast_t* arg = ast->children[0]->children[i];
					if (arg->value.type == LEX_TYPE_COLON)
					{
						set_llvm_local(env, arg->children[0]->value.value, NULL);
					}
				}
			}

			// Check value of the assignment
			if (ast->value.type == LEX_TYPE_ASSIGN)
				find_llvm_closure_locals(env, ast->children[1], closed_locals);
		}

		// Check value of scope
		find_llvm_closure_locals(env, body->children[body->children_count - 1], closed_locals);

		// Pop scope
		env->local = pop_llvm_scope(env->local);
	} else
	{
		// Find locals closed over in child nodes
		for (size_t i = 0; i < body->children_count; i++)
		{
			find_llvm_closure_locals(env, body->children[i], closed_locals);
		}
	}
}

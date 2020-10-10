// 
// llvm
// environment.c: Creates an environment for LLVM IR codegen.
// 
// Created by jenra.
// Created on September 29 2020.
// 

#include "environment.h"

// push_llvm_scope(llvm_scope_t*) -> llvm_scope_t*
// Pushes an LLVM scope onto the stack of scopes.
llvm_scope_t* push_llvm_scope(llvm_scope_t* parent)
{
	llvm_scope_t* scope = malloc(sizeof(llvm_scope_t));
	scope->parent = parent;
	scope->variables = init_hashmap();
	scope->parameters = init_hashmap();
	return scope;
}

// pop_llvm_scope(llvm_scope_t*) -> llvm_scope_t*
// Pops an LLVM scope onto the stack of scopes.
llvm_scope_t* pop_llvm_scope(llvm_scope_t* scope)
{
	llvm_scope_t* parent = scope->parent;
	del_hashmap(scope->variables);
	del_hashmap(scope->parameters);
	return parent;
}

// create_llvm_codegen_environment(LLVMModuleRef) -> llvm_codegen_env_t*
// Creates an LLVM codegen environment.
llvm_codegen_env_t* create_llvm_codegen_environment(LLVMModuleRef header_mod)
{
	llvm_codegen_env_t* env = malloc(sizeof(llvm_codegen_env_t));
	env->local = NULL;
	env->header_mod = header_mod;
	env->body_mod = NULL;
	env->main_func = NULL;
	env->current_func = NULL;
	env->current_block = NULL;

	// Create necessary types
	if (LLVMGetTypeByName(header_mod, "func.app.type") == NULL)
	{
		// Function application structure
		// typedef struct s_func_app
		// {
		//	union
		//	{
		//		int32_t reference_count;
		//		int64_t i64;
		//		double f64;
		//	};
		//
		// 	void* func;
		// 	int8_t count;
		// 	int64_t thunk_bitmap;
		// 	struct s_func_app* args;
		// } func_app_t;
		LLVMContextRef context = LLVMGetModuleContext(header_mod);
		LLVMTypeRef func_app = LLVMStructCreateNamed(context, "func.app.type");
		LLVMTypeRef func_app_body[] = {LLVMInt32Type(), LLVMPointerType(LLVMInt64Type(), 0), LLVMInt8Type(), LLVMInt8Type(), LLVMInt64Type(), LLVMPointerType(func_app, 0)};
		LLVMStructSetBody(func_app, func_app_body, 6, false);
	}
	return env;
}

// set_llvm_local(llvm_codegen_env_t*, char*, LLVMValueRef) -> void
// Sets a local to the scope.
void set_llvm_local(llvm_codegen_env_t* env, char* local, LLVMValueRef value)
{
	if (env->local != NULL)
		map_add(env->local->variables, local, value);
}

// set_llvm_param(llvm_codegen_env_t*, char*, LLVMValueRef) -> void
// Sets a parameter to the scope.
void set_llvm_param(llvm_codegen_env_t* env, char* local, LLVMValueRef value, uint8_t param_index)
{
	if (env->local != NULL)
	{
		map_add(env->local->variables, local, value);
		map_add(env->local->parameters, local, (void*) (uint64_t) param_index);
	}
}

// lookup_llvm_local(llvm_codegen_env_t*, char*) -> LLVMValueRef
// Looks up a local and returns its LLVMValueRef if available.
LLVMValueRef lookup_llvm_local(llvm_codegen_env_t* env, char* local)
{
	// Get most local scope
	llvm_scope_t* scope = env->local;

	// Iterate over all scopes in the linked list
	while (scope != NULL)
	{
		// Get the value
		LLVMValueRef value = map_get(scope->variables, local);
		if (value != NULL)
			return value;

		// Parent scope
		scope = scope->parent;
	}

	// Local not found
	return NULL;
}

// lookup_llvm_param(llvm_codegen_env_t*, char*) -> uint64_t
// Looks up a parameter and returns its parameter index if available.
uint64_t lookup_llvm_param(llvm_codegen_env_t* env, char* local)
{
	// Get most local scope
	llvm_scope_t* scope = env->local;

	// Iterate over all scopes in the linked list
	while (scope != NULL)
	{
		// Get the parameter index
		if (map_contains(scope->parameters, local))
			return (uint64_t) map_get(scope->parameters, local);

		// Parent scope
		scope = scope->parent;
	}

	// Parameter not found
	return (uint64_t) -1;
}

// empty_llvm_codegen_environment(llvm_codegen_env_t*) -> void
// Emptys an LLVM codegen environment for reuse.
void empty_llvm_codegen_environment(llvm_codegen_env_t* env)
{
	while (env->local != NULL)
	{
		env->local = pop_llvm_scope(env->local);
	}
	env->body_mod = NULL;
	env->main_func = NULL;
	env->current_func = NULL;
	env->current_block = NULL;
}

// clean_llvm_codegen_environment(llvm_codegen_env_t)
// Cleans an LLVM codegen environment.
void clean_llvm_codegen_environment(llvm_codegen_env_t* env)
{
	if (env->header_mod != NULL && env->header_mod != env->body_mod)
		LLVMDisposeModule(env->header_mod);
	while (env->local != NULL)
	{
		env->local = pop_llvm_scope(env->local);
	}
	free(env);
}

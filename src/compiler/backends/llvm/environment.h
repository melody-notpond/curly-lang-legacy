// 
// llvm
// environment.h: Header file for environment.c.
// 
// Created by jenra.
// Created on September 29 2020.
// 

#ifndef LLVM_ENVIRONMENT_H
#define LLVM_ENVIRONMENT_H

#include "llvm-c/Core.h"

#include "../../../utils/hashmap.h"

// Represents a scope
typedef struct s_llvm_scope
{
	hashmap_t* variables;

	struct s_llvm_scope* parent;
} llvm_scope_t;

typedef struct
{
	llvm_scope_t* local;

	LLVMModuleRef header_mod;

	LLVMModuleRef body_mod;

	LLVMValueRef main_func;

	LLVMValueRef current_func;

	LLVMBasicBlockRef current_block;
} llvm_codegen_env_t;

// push_llvm_scope(llvm_scope_t*) -> llvm_scope_t*
// Pushes an LLVM scope onto the stack of scopes.
llvm_scope_t* push_llvm_scope(llvm_scope_t* parent);

// pop_llvm_scope(llvm_scope_t*) -> llvm_scope_t*
// Pops an LLVM scope onto the stack of scopes.
llvm_scope_t* pop_llvm_scope(llvm_scope_t* scope);

// create_llvm_codegen_environment(LLVMModuleRef) -> llvm_codegen_env_t*
// Creates an LLVM codegen environment.
llvm_codegen_env_t* create_llvm_codegen_environment(LLVMModuleRef header_mod);

// lookup_llvm_local(llvm_codegen_env_t*, char*) -> LLVMValueRef
// Looks up a local and return its LLVMValueRef if available.
LLVMValueRef lookup_llvm_local(llvm_codegen_env_t* env, char* local);

// empty_llvm_codegen_environment(llvm_codegen_env_t*) -> void
// Emptys an LLVM codegen environment for reuse.
void empty_llvm_codegen_environment(llvm_codegen_env_t* env);

// clean_llvm_codegen_environment(llvm_codegen_env_t)
// Cleans an LLVM codegen environment.
void clean_llvm_codegen_environment(llvm_codegen_env_t* env);

#endif /* LLVM_ENVIRONMENT_H */

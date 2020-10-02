// 
// llvm
// functions.h: Header file for functions.c.
// 
// Created by jenra.
// Created on October 2 2020.
// 

#ifndef LLVM_FUNCTIONS_H
#define LLVM_FUNCTIONS_H

#include <llvm-c/Core.h>

#include "../../frontend/parse/ast.h"
#include "../../../utils/hashmap.h"
#include "environment.h"

// find_llvm_closure_locals(llvm_codegen_env_t*, ast_t*, hashmap_t*) -> void
// Finds all locals the function closes over and adds them to the hash of closed locals.
void find_llvm_closure_locals(llvm_codegen_env_t* env, ast_t* body, hashmap_t* closed_locals);

#endif /* LLVM_FUNCTIONS_H */

// 
// llvm
// llvm_types.h: Header file for llvm_types.c.
// 
// Created by jenra.
// Created on September 29 2020.
// 

#ifndef LLVM_TYPES_H
#define LLVM_TYPES_H

#include <llvm-c/Core.h>

#include "../../frontend/parse/ast.h"

// internal_type_to_llvm(ast_t*) -> LLVMTypeRef
// Converts an internal type into an LLVM IR type.
LLVMTypeRef internal_type_to_llvm(ast_t* ast);

#endif /* LLVM_TYPES_H */

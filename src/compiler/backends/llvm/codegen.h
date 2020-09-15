//
// Curly
// codegen.h: Header file for codegen.c.
//
// Created by jenra.
// Created on September 7 2020.
//

#ifndef LLVM_CODEGEN_H
#define LLVM_CODEGEN_H

#include <llvm-c/Core.h>

#include "../../frontend/parse/ast.h"

// generate_code(ast_t*, LLVMModuleRef) -> LLVMValueRef
// Generates llvm ir code from an ast.
LLVMValueRef generate_code(ast_t* ast, LLVMModuleRef mod);

#endif /* LLVM_CODEGEN_H */

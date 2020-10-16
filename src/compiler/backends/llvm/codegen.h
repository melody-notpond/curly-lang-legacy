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

#include "environment.h"
#include "../../frontend/ir/generate_ir.h"

// generate_code(curly_ir_t, llvm_codegen_env_t*) -> llvm_codegen_env_t*
// Generates llvm ir code from an ast.
llvm_codegen_env_t* generate_code(curly_ir_t ir, llvm_codegen_env_t* env);

#endif /* LLVM_CODEGEN_H */

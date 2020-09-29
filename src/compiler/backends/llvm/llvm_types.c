// 
// llvm
// llvm_types.c: Defines a function for converting internal types to LLVM IR types.
// 
// Created by jenra.
// Created on September 29 2020.
// 

#include <string.h>

#include "llvm_types.h"

// internal_type_to_llvm(ast_t*) -> LLVMTypeRef
// Converts an internal type into an LLVM IR type.
LLVMTypeRef internal_type_to_llvm(ast_t* ast)
{
	type_t* type = ast->type;
	if (type->type_type == IR_TYPES_PRIMITIVE && !strcmp(type->type_name, "Int"))
		return LLVMInt64Type();
	else if (type->type_type == IR_TYPES_PRIMITIVE && !strcmp(type->type_name, "Float"))
		return LLVMDoubleType();
	else if (type->type_type == IR_TYPES_PRIMITIVE && !strcmp(type->type_name, "Bool"))
		return LLVMInt1Type();
	else return LLVMVoidType();
}

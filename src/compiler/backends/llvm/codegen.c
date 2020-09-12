//
// Curly
// codegen.c: Implements code generation to LLVM IR.
//
// Created by jenra.
// Created on September 7 2020.
//

#include <string.h>

#include "codegen.h"

// build_expression(ast_t*, LLVMModuleRef, LLVMBuilderRef) -> LLVMValueRef
// Builds an expression to LLVM IR.
LLVMValueRef build_expression(ast_t* ast, LLVMModuleRef mod, LLVMBuilderRef builder);

// build_infix(ast_t*, LLVMModuleRef, LLVMBuilderRef) -> LLVMValueRef
// Builds an infix expression to LLVM IR.
LLVMValueRef build_infix(ast_t* ast, LLVMModuleRef mod, LLVMBuilderRef builder)
{
	// Build the operands
	LLVMValueRef left = build_expression(ast->children[0], mod, builder);
	LLVMValueRef right = build_expression(ast->children[1], mod, builder);

	// Cast ints to floats if necessary
	type_t* ltype = ast->children[0]->type;
	type_t* rtype = ast->children[1]->type;
	if (ltype->type_type == IR_TYPES_PRIMITIVE && rtype->type_type == IR_TYPES_PRIMITIVE)
	{
		if (!strcmp(ltype->type_name, "Int") && !strcmp(rtype->type_name, "Float"))
			left = LLVMBuildCast(builder, LLVMSIToFP, left, LLVMDoubleType(), "");
		else if (!strcmp(ltype->type_name, "Float") && !strcmp(rtype->type_name, "Int"))
			right = LLVMBuildCast(builder, LLVMSIToFP, right, LLVMDoubleType(), "");
	}

	// Build the operator
	switch(ast->value.value[0])
	{
		case '*':
			if (ast->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->type->type_name, "Int"))
				return LLVMBuildMul(builder, left, right, "");
			else return LLVMBuildFMul(builder, left, right, "");
		case '/':
			if (ast->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->type->type_name, "Int"))
				return LLVMBuildSDiv(builder, left, right, "");
			else return LLVMBuildFDiv(builder, left, right, "");
		case '%':
			return LLVMBuildSRem(builder, left, right, "");
		case '+':
			if (ast->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->type->type_name, "Int"))
				return LLVMBuildAdd(builder, left, right, "");
			else return LLVMBuildFAdd(builder, left, right, "");
		case '-':
			if (ast->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->type->type_name, "Int"))
				return LLVMBuildSub(builder, left, right, "");
			else return LLVMBuildFSub(builder, left, right, "");
		case '<':
			return LLVMBuildShl(builder, left, right, "");
		case '>':
			return LLVMBuildLShr(builder, left, right, "");
		case '&':
			return LLVMBuildAnd(builder, left, right, "");
		case '|':
			return LLVMBuildOr(builder, left, right, "");
		case '^':
			return LLVMBuildXor(builder, left, right, "");
		default:
			return NULL;
	}
}

// build_expression(ast_t*, LLVMModuleRef, LLVMBuilderRef) -> LLVMValueRef
// Builds an expression to LLVM IR.
LLVMValueRef build_expression(ast_t* ast, LLVMModuleRef mod, LLVMBuilderRef builder)
{
	// Build operand (currently only integers are supported)
	if (ast->value.tag == LEX_TAG_OPERAND && ast->value.type != LEX_TYPE_SYMBOL)
	{
		switch (ast->value.type)
		{
			case LEX_TYPE_INT:
				return LLVMConstInt(LLVMInt64Type(), atoll(ast->value.value), false);
			case LEX_TYPE_FLOAT:
				return LLVMConstReal(LLVMDoubleType(), atof(ast->value.value));
			default:
				return NULL;
		}

	// Build infix expression
	} else if (ast->value.tag == LEX_TAG_INFIX_OPERATOR)
		return build_infix(ast, mod, builder);
	return NULL;
}

// get_main_ret_type(ast_t*) -> LLVMTypeRef
// Gets the return type for the main function.
LLVMTypeRef get_main_ret_type(ast_t* ast)
{
	type_t* type = ast->children[ast->children_count - 1]->type;
	if (type->type_type == IR_TYPES_PRIMITIVE && !strcmp(type->type_name, "Int"))
		return LLVMInt64Type();
	else if (type->type_type == IR_TYPES_PRIMITIVE && !strcmp(type->type_name, "Float"))
		return LLVMDoubleType();
	else return NULL;
}

// generate_code(ast_t*) -> LLVMModuleRef
// Generates llvm ir code from an ast.
LLVMModuleRef generate_code(ast_t* ast)
{
	// Create the main function
	LLVMModuleRef mod = LLVMModuleCreateWithName("curly_main");
	LLVMTypeRef main_type = LLVMFunctionType(get_main_ret_type(ast), (LLVMTypeRef[]) {}, 0, false);
	LLVMValueRef main = LLVMAddFunction(mod, "main", main_type);

	// Create the entry basic block
	LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "entry");
	LLVMBuilderRef builder = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(builder, entry);

	// Iterate over every element if the node is the topmost parent
	LLVMValueRef value = NULL;
	for (size_t i = 0; i < ast->children_count; i++)
	{
		value = build_expression(ast->children[i], mod, builder);
	}

	// Create a return instruction
	LLVMBuildRet(builder, value);
	LLVMDisposeBuilder(builder);
	return mod;
}

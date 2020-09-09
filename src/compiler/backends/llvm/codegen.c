//
// Curly
// codegen.c: Implements code generation to LLVM IR.
//
// Created by jenra.
// Created on September 7 2020.
//

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

	// Build the operator
	switch(ast->value.value[0])
	{
		case '*':
			return LLVMBuildMul(builder, left, right, "");
		case '/':
			return LLVMBuildSDiv(builder, left, right, "");
		case '%':
			return LLVMBuildSRem(builder, left, right, "");
		case '+':
			return LLVMBuildAdd(builder, left, right, "");
		case '-':
			return LLVMBuildSub(builder, left, right, "");
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
			default:
				return NULL;
		}

	// Build infix expression
	} else if (ast->value.tag == LEX_TAG_INFIX_OPERATOR)
		return build_infix(ast, mod, builder);
	return NULL;
}

// generate_code(ast_t*) -> LLVMModuleRef
// Generates llvm ir code from an ast.
LLVMModuleRef generate_code(ast_t* ast)
{
	// Create the main function
	LLVMModuleRef mod = LLVMModuleCreateWithName("curly_main");
	LLVMTypeRef main_type = LLVMFunctionType(LLVMInt64Type(), (LLVMTypeRef[]) {}, 0, false);
	LLVMValueRef main = LLVMAddFunction(mod, "main", main_type);

	// Create the entry basic block
	LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "entry");
	LLVMBuilderRef builder = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(builder, entry);

	// Iterate over every element if the node is the topmost parent
	LLVMValueRef value = NULL;
	if (ast->value.value == NULL)
	{
		for (size_t i = 0; i < ast->children_count; i++)
		{
			value = build_expression(ast->children[i], mod, builder);
		}

	// Build the ast node if it's not the topmost parent
	} else value = build_expression(ast, mod, builder);

	// Create a return instruction
	LLVMBuildRet(builder, value);
	LLVMDisposeBuilder(builder);
	return mod;
}

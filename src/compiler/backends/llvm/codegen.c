//
// Curly
// codegen.c: Implements code generation to LLVM IR.
//
// Created by jenra.
// Created on September 7 2020.
//

#include <string.h>

#include "codegen.h"

// build_expression(ast_t*, LLVMModuleRef, LLVMBuilderRef, hashmap_t*) -> LLVMValueRef
// Builds an expression to LLVM IR.
LLVMValueRef build_expression(ast_t* ast, LLVMModuleRef mod, LLVMBuilderRef builder, hashmap_t* locals);

// build_assignment(ast_t*, LLVMModuleRef, LLVMBuilderRef, bool, hashmap_t*) -> LLVMValueRef
// Builds an assignment to LLVM IR.
LLVMValueRef build_assignment(ast_t* ast, LLVMModuleRef mod, LLVMBuilderRef builder, bool global, hashmap_t* locals);

// build_infix(ast_t*, LLVMModuleRef, LLVMBuilderRef, hashmap_t*) -> LLVMValueRef
// Builds an infix expression to LLVM IR.
LLVMValueRef build_infix(ast_t* ast, LLVMModuleRef mod, LLVMBuilderRef builder, hashmap_t* locals)
{
	// Build the operands
	LLVMValueRef left = build_expression(ast->children[0], mod, builder, locals);
	LLVMValueRef right = build_expression(ast->children[1], mod, builder, locals);

	// Cast ints to floats if necessary
	type_t* ltype = ast->children[0]->type;
	type_t* rtype = ast->children[1]->type;
	if (ltype->type_type == IR_TYPES_PRIMITIVE && rtype->type_type == IR_TYPES_PRIMITIVE)
	{
		if (!strcmp(ltype->type_name, "Int") && !strcmp(rtype->type_name, "Float"))
			left = LLVMBuildSIToFP(builder, left, LLVMDoubleType(), "");
		else if (!strcmp(ltype->type_name, "Float") && !strcmp(rtype->type_name, "Int"))
			right = LLVMBuildSIToFP(builder, right, LLVMDoubleType(), "");
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

// build_expression(ast_t*, LLVMModuleRef, LLVMBuilderRef, hashmap_t*) -> LLVMValueRef
// Builds an expression to LLVM IR.
LLVMValueRef build_expression(ast_t* ast, LLVMModuleRef mod, LLVMBuilderRef builder, hashmap_t* locals)
{
	// Build operand
	if (ast->value.tag == LEX_TAG_OPERAND)
	{
		switch (ast->value.type)
		{
			case LEX_TYPE_INT:
				return LLVMConstInt(LLVMInt64Type(), atoll(ast->value.value), false);
			case LEX_TYPE_FLOAT:
				return LLVMConstReal(LLVMDoubleType(), atof(ast->value.value));
			case LEX_TYPE_SYMBOL:
			{
				LLVMValueRef local = map_get(locals, ast->value.value);
				if (local != NULL)
					return local;
				LLVMValueRef global = LLVMGetNamedGlobal(mod, ast->value.value);
				return LLVMBuildLoad(builder, global, "");
			}
			default:
				return NULL;
		}

	// Build infix expression
	} else if (ast->value.tag == LEX_TAG_INFIX_OPERATOR)
		return build_infix(ast, mod, builder, locals);

	// Build with expressions
	else if (!strcmp(ast->value.value, "with"))
	{
		// Build all the assignments
		for (size_t i = 0; i < ast->children_count - 1; i++)
		{
			if (ast->children[i]->value.type == LEX_TYPE_ASSIGN)
				build_assignment(ast->children[i], mod, builder, false, locals);
		}

		// Build the scope
		LLVMValueRef value = build_expression(ast->children[ast->children_count - 1], mod, builder, locals);


		// Remove locals from the local scope
		for (size_t i = 0; i < ast->children_count - 1; i++)
		{
			if (ast->children[i]->value.type == LEX_TYPE_ASSIGN)
			{
				// var = expr and var: type = expr
				char* name = NULL;
				if (ast->children[0]->value.type == LEX_TYPE_SYMBOL && ast->children[0]->children_count == 0)
					name = ast->children[0]->value.value;
				else if (ast->children[0]->value.type == LEX_TYPE_RANGE)
					name = ast->children[0]->children[0]->value.value;

				map_remove(locals, name);
			}
		}

		return value;
	} else return NULL;
}

// build_assignment(ast_t*, LLVMModuleRef, LLVMBuilderRef, bool, hashmap_t*) -> LLVMValueRef
// Builds an assignment to LLVM IR.
LLVMValueRef build_assignment(ast_t* ast, LLVMModuleRef mod, LLVMBuilderRef builder, bool global, hashmap_t* locals)
{
	// var = expr and var: type = expr
	char* name = NULL;
	if (ast->children[0]->value.type == LEX_TYPE_SYMBOL && ast->children[0]->children_count == 0)
		name = ast->children[0]->value.value;
	else if (ast->children[0]->value.type == LEX_TYPE_RANGE)
		name = ast->children[0]->children[0]->value.value;

	if (global)
	{
		// Set the global variable
		if (name != NULL)
		{
			LLVMValueRef value = build_expression(ast->children[1], mod, builder, locals);
			LLVMValueRef global = LLVMGetNamedGlobal(mod, name);
			if (global == NULL)
				global = LLVMAddGlobal(mod, LLVMTypeOf(value), name);
			LLVMSetInitializer(global, LLVMConstInt(LLVMInt64Type(), 0, false));
			LLVMSetLinkage(global, LLVMPrivateLinkage);
			LLVMBuildStore(builder, value, global);
			return value;
		}
	} else
	{
		LLVMValueRef local = build_expression(ast->children[1], mod, builder, locals);
		map_add(locals, name, local);
		return local;
	}

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

// generate_code(ast_t*, LLVMModuleRef) -> LLVMModuleRef
// Generates llvm ir code from an ast.
LLVMModuleRef generate_code(ast_t* ast, LLVMModuleRef mod)
{
	// Create the main module
	LLVMContextRef context;
	LLVMModuleRef main_mod;
	if (mod != NULL)
	{
		context = LLVMGetModuleContext(mod);
		main_mod = LLVMModuleCreateWithNameInContext("stdin", context);
	} else
	{
		context = LLVMContextCreate();
		main_mod = LLVMModuleCreateWithNameInContext("file", context);
		mod = main_mod;
	}

	// Create the main function
	LLVMTypeRef main_type = LLVMFunctionType(get_main_ret_type(ast), (LLVMTypeRef[]) {}, 0, false);
	LLVMValueRef main = LLVMAddFunction(main_mod, "main", main_type);

	// Create the entry basic block
	LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "entry");
	LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);
	LLVMPositionBuilderAtEnd(builder, entry);

	// Create a dictionary for locals
	hashmap_t* locals = init_hashmap();

	// Iterate over every element of the topmost parent and build
	LLVMValueRef value = NULL;
	for (size_t i = 0; i < ast->children_count; i++)
	{
		if (ast->children[i]->value.type == LEX_TYPE_ASSIGN)
			value = build_assignment(ast->children[i], mod, builder, true, locals);
		else if (ast->children[i]->value.type != LEX_TYPE_COLON)
			value = build_expression(ast->children[i], mod, builder, locals);
	}

	// Create a return instruction
	del_hashmap(locals);
	LLVMBuildRet(builder, value);
	LLVMDisposeBuilder(builder);
	return main_mod;
}

//
// Curly
// codegen.c: Implements code generation to LLVM IR.
//
// Created by jenra.
// Created on September 7 2020.
//

#include <string.h>

#include "codegen.h"

// build_expression(ast_t*, LLVMBuilderRef, llvm_codegen_env_t*) -> LLVMValueRef
// Builds an expression to LLVM IR.
LLVMValueRef build_expression(ast_t* ast, LLVMBuilderRef builder, llvm_codegen_env_t* env);

// build_assignment(ast_t*, LLVMBuilderRef, llvm_codegen_env_t*) -> LLVMValueRef
// Builds an assignment to LLVM IR.
LLVMValueRef build_assignment(ast_t* ast, LLVMBuilderRef builder, llvm_codegen_env_t* env);

// build_infix(ast_t*, LLVMBuilderRef, llvm_codegen_env_t*) -> LLVMValueRef
// Builds an infix expression to LLVM IR.
LLVMValueRef build_infix(ast_t* ast, LLVMBuilderRef builder, llvm_codegen_env_t* env)
{
	// Build the operands
	LLVMValueRef left = build_expression(ast->children[0], builder, env);
	LLVMValueRef right = build_expression(ast->children[1], builder, env);

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
			if (ast->value.value[1] == '<')
				return LLVMBuildShl(builder, left, right, "");
			else if (ast->value.value[1] == '\0')
			{
				if (ast->children[0]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[0]->type->type_name, "Int")
				 && ast->children[1]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[1]->type->type_name, "Int"))
					return LLVMBuildICmp(builder, LLVMIntSLT, left, right, "");
				else return LLVMBuildFCmp(builder, LLVMRealOLT, left, right, "");
			} else if (ast->value.value[1] == '=')
			{
				if (ast->children[0]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[0]->type->type_name, "Int")
				 && ast->children[1]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[1]->type->type_name, "Int"))
					return LLVMBuildICmp(builder, LLVMIntSLE, left, right, "");
				else return LLVMBuildFCmp(builder, LLVMRealOLE, left, right, "");
			} else return NULL;
		case '>':
			if (ast->value.value[1] == '>')
				return LLVMBuildLShr(builder, left, right, "");
			else if (ast->value.value[1] == '\0')
			{
				if (ast->children[0]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[0]->type->type_name, "Int")
				 && ast->children[1]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[1]->type->type_name, "Int"))
					return LLVMBuildICmp(builder, LLVMIntSGT, left, right, "");
				else return LLVMBuildFCmp(builder, LLVMRealOGT, left, right, "");
			} else if (ast->value.value[1] == '=')
			{
				if (ast->children[0]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[0]->type->type_name, "Int")
				 && ast->children[1]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[1]->type->type_name, "Int"))
					return LLVMBuildICmp(builder, LLVMIntSGE, left, right, "");
				else return LLVMBuildFCmp(builder, LLVMRealOGE, left, right, "");
			} else return NULL;
		case '&':
			return LLVMBuildAnd(builder, left, right, "");
		case '|':
			return LLVMBuildOr(builder, left, right, "");
		case '^':
		case 'x':
			return LLVMBuildXor(builder, left, right, "");
		case '=':
			if (ast->children[0]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[0]->type->type_name, "Int")
				 && ast->children[1]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[1]->type->type_name, "Int"))
				return LLVMBuildICmp(builder, LLVMIntEQ, left, right, "");
			else return LLVMBuildFCmp(builder, LLVMRealOEQ, left, right, "");
		case '!':
			if (ast->children[0]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[0]->type->type_name, "Int")
				 && ast->children[1]->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ast->children[1]->type->type_name, "Int"))
				return LLVMBuildICmp(builder, LLVMIntNE, left, right, "");
			else return LLVMBuildFCmp(builder, LLVMRealONE, left, right, "");
		default:
			return NULL;
	}
}

// build_expression(ast_t*, LLVMBuilderRef, llvm_codegen_env_t*) -> LLVMValueRef
// Builds an expression to LLVM IR.
LLVMValueRef build_expression(ast_t* ast, LLVMBuilderRef builder, llvm_codegen_env_t* env)
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
			case LEX_TYPE_BOOL:
				return LLVMConstInt(LLVMInt1Type(), !strcmp(ast->value.value, "true"), false);
			case LEX_TYPE_SYMBOL:
			{
				LLVMValueRef local = lookup_llvm_local(env, ast->value.value);
				if (local != NULL)
					return local;
				LLVMValueRef global = LLVMGetNamedGlobal(env->header_mod, ast->value.value);
				return LLVMBuildLoad(builder, global, "");
			}
			default:
				return NULL;
		}

	// Build and
	} else if (!strcmp(ast->value.value, "and"))
	{
		// Build the left operand
		LLVMValueRef left = build_expression(ast->children[0], builder, env);

		// Create basic blocks to jump to
		LLVMBasicBlockRef from = env->current_block;
		LLVMBasicBlockRef right_block = LLVMAppendBasicBlock(env->current_func, "and.rhs");
		LLVMMoveBasicBlockAfter(right_block, from);
		LLVMBasicBlockRef post_and = LLVMAppendBasicBlock(env->current_func, "and.post");
		LLVMMoveBasicBlockAfter(post_and, right_block);

		// Build the branch and right operand
		LLVMBuildCondBr(builder, left, right_block, post_and);
		LLVMPositionBuilderAtEnd(builder, right_block);
		env->current_block = right_block;
		LLVMValueRef right = build_expression(ast->children[1], builder, env);

		// Build phi
		LLVMBuildBr(builder, post_and);
		LLVMPositionBuilderAtEnd(builder, post_and);
		env->current_block = post_and;
		LLVMValueRef phi = LLVMBuildPhi(builder, LLVMInt1Type(), "");
		LLVMValueRef incoming_values[] = {left, right};
		LLVMBasicBlockRef incoming_blocks[] = {LLVMGetPreviousBasicBlock(right_block), LLVMGetPreviousBasicBlock(post_and)};
		LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);
		return phi;

	// Build or
	} else if (!strcmp(ast->value.value, "or"))
	{
		// Build the left operand
		LLVMValueRef left = build_expression(ast->children[0], builder, env);

		// Create basic blocks to jump to
		LLVMBasicBlockRef from = env->current_block;
		LLVMBasicBlockRef right_block = LLVMAppendBasicBlock(env->current_func, "or.rhs");
		LLVMMoveBasicBlockAfter(right_block, from);
		LLVMBasicBlockRef post_or = LLVMAppendBasicBlock(env->current_func, "or.post");
		LLVMMoveBasicBlockAfter(post_or, right_block);

		// Build the branch and right operand
		LLVMBuildCondBr(builder, left, post_or, right_block);
		LLVMPositionBuilderAtEnd(builder, right_block);
		env->current_block = right_block;
		LLVMValueRef right = build_expression(ast->children[1], builder, env);

		// Build phi
		LLVMBuildBr(builder, post_or);
		LLVMPositionBuilderAtEnd(builder, post_or);
		env->current_block = post_or;
		LLVMValueRef phi = LLVMBuildPhi(builder, LLVMInt1Type(), "");
		LLVMValueRef incoming_values[] = {left, right};
		LLVMBasicBlockRef incoming_blocks[] = {LLVMGetPreviousBasicBlock(right_block), LLVMGetPreviousBasicBlock(post_or)};
		LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);
		return phi;

	// Build infix expression
	} else if (ast->value.tag == LEX_TAG_INFIX_OPERATOR)
		return build_infix(ast, builder, env);

	// Build unary minus
	else if (!strcmp(ast->value.value, "-"))
	{
		LLVMValueRef operand = build_expression(ast->children[0], builder, env);
		return LLVMBuildNeg(builder, operand, "");

	// Build with expressions
	} else if (!strcmp(ast->value.value, "with"))
	{
		// Push local scope
		env->local = push_llvm_scope(env->local);

		// Build all the assignments
		for (size_t i = 0; i < ast->children_count - 1; i++)
		{
			if (ast->children[i]->value.type == LEX_TYPE_ASSIGN)
				build_assignment(ast->children[i], builder, env);
		}

		// Build the scope
		LLVMValueRef value = build_expression(ast->children[ast->children_count - 1], builder, env);

		// Pop local scope
		env->local = pop_llvm_scope(env->local);
		return value;
	} else if (!strcmp(ast->value.value, "if"))
	{
		// Build condition
		LLVMValueRef cond = build_expression(ast->children[0], builder, env);

		// Create basic blocks to jump to
		LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(env->current_func, "if.then");
		LLVMMoveBasicBlockAfter(then_block, env->current_block);
		LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(env->current_func, "if.else");
		LLVMMoveBasicBlockAfter(else_block, then_block);
		LLVMBasicBlockRef post_if = LLVMAppendBasicBlock(env->current_func, "if.post");
		LLVMMoveBasicBlockAfter(post_if, else_block);

		// Build the branch and then body
		LLVMBuildCondBr(builder, cond, then_block, else_block);
		LLVMPositionBuilderAtEnd(builder, then_block);
		env->current_block = then_block;
		LLVMValueRef then_val = build_expression(ast->children[1], builder, env);

		// Build the branch and else body
		LLVMBuildBr(builder, post_if);
		LLVMPositionBuilderAtEnd(builder, else_block);
		env->current_block = else_block;
		LLVMValueRef else_val = build_expression(ast->children[2], builder, env);

		// Build phi
		LLVMBuildBr(builder, post_if);
		LLVMPositionBuilderAtEnd(builder, post_if);
		env->current_block = post_if;
		LLVMValueRef phi = LLVMBuildPhi(builder, LLVMInt1Type(), "");
		LLVMValueRef incoming_values[] = {then_val, else_val};
		LLVMBasicBlockRef incoming_blocks[] = {LLVMGetPreviousBasicBlock(else_block), LLVMGetPreviousBasicBlock(post_if)};
		LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);
		return phi;
	} else return NULL;
}

// build_assignment(ast_t*, LLVMBuilderRef, llvm_codegen_env_t*) -> LLVMValueRef
// Builds an assignment to LLVM IR.
LLVMValueRef build_assignment(ast_t* ast, LLVMBuilderRef builder, llvm_codegen_env_t* env)
{
	// var = expr and var: type = expr
	char* name = NULL;
	if (ast->children[0]->value.type == LEX_TYPE_SYMBOL && ast->children[0]->children_count == 0)
		name = ast->children[0]->value.value;
	else if (ast->children[0]->value.type == LEX_TYPE_COLON)
		name = ast->children[0]->children[0]->value.value;

	// New variable (typed or untyped)
	if (name != NULL)
	{
		// Set the global variable
		if (env->local == NULL)
		{
			// Get expression and global
			LLVMValueRef value = build_expression(ast->children[1], builder, env);
			LLVMValueRef global = LLVMGetNamedGlobal(env->header_mod, name);
			size_t length = 0;

			// Create missing global
			if (global == NULL)
			{
				global = LLVMAddGlobal(env->header_mod, LLVMTypeOf(value), name);
				if (!strcmp(LLVMGetModuleIdentifier(env->header_mod, &length), "repl-header"))
					LLVMSetLinkage(global, LLVMExternalWeakLinkage);
				else
				{
					LLVMSetLinkage(global, LLVMCommonLinkage);
					LLVMSetInitializer(global, LLVMConstInt(LLVMInt64Type(), 0, false));
				}
			}

			// Build store instruction
			LLVMBuildStore(builder, value, global);
			return value;
		} else
		{
			LLVMValueRef local = build_expression(ast->children[1], builder, env);
			map_add(env->local->variables, name, local);
			return local;
		}

	// Functions
	} else if (ast->children[0]->value.type == LEX_TYPE_SYMBOL && ast->children[0]->children_count > 0)
	{
		LLVMValueRef ret = build_expression(ast->children[1], builder, env);
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
	else if (type->type_type == IR_TYPES_PRIMITIVE && !strcmp(type->type_name, "Bool"))
		return LLVMInt1Type();
	else return LLVMVoidType();
}

// generate_code(ast_t*, llvm_codegen_env_t*) -> llvm_codegen_env_t*
// Generates llvm ir code from an ast.
llvm_codegen_env_t* generate_code(ast_t* ast, llvm_codegen_env_t* env)
{
	LLVMContextRef context;
	if (env == NULL)
	{
		// Create the main module
		context = LLVMContextCreate();
		LLVMModuleRef main_mod = LLVMModuleCreateWithNameInContext("file", context);
		env = create_llvm_codegen_environment(main_mod);
		env->body_mod = main_mod;
	} else
	{
		context = LLVMGetModuleContext(env->header_mod);
		env->body_mod = LLVMModuleCreateWithNameInContext("stdin", context);
	}

	// Create the main function
	LLVMTypeRef main_type = LLVMFunctionType(get_main_ret_type(ast), (LLVMTypeRef[]) {}, 0, false);
	env->main_func = LLVMAddFunction(env->body_mod, "main", main_type);
	env->current_func = env->main_func;

	// Create the entry basic block
	env->current_block = LLVMAppendBasicBlock(env->current_func, "entry");
	LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);
	LLVMPositionBuilderAtEnd(builder, env->current_block);

	// Iterate over every element of the topmost parent and build
	LLVMValueRef value = NULL;
	for (size_t i = 0; i < ast->children_count; i++)
	{
		if (ast->children[i]->value.type == LEX_TYPE_ASSIGN)
			value = build_assignment(ast->children[i], builder, env);
		else if (ast->children[i]->value.type != LEX_TYPE_COLON)
			value = build_expression(ast->children[i], builder, env);
	}

	// Create a return instruction
	LLVMBuildRet(builder, value);
	LLVMDisposeBuilder(builder);
	return env;
}

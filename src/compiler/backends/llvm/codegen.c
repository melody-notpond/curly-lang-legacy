//
// Curly
// codegen.c: Implements code generation to LLVM IR.
//
// Created by jenra.
// Created on September 7 2020.
//

#include <string.h>

#include "codegen.h"
#include "functions.h"
#include "llvm_types.h"

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
				// Get local
				LLVMValueRef local = lookup_llvm_local(env, ast->value.value);
				if (local != NULL)
				{
					// Local is definitely not a thunk
					uint64_t param_index = lookup_llvm_param(env, ast->value.value);
					if (param_index >= 64)
						return local;

					// Local is a parameter that has not been checked for thunkiness
					LLVMValueRef thunk_bitmap_ptr = LLVMGetParam(env->current_func, 0);
					LLVMValueRef thunk_bitmap = LLVMBuildLoad2(builder, LLVMInt64Type(), thunk_bitmap_ptr, "thunk.bitmap.deref");
					LLVMValueRef arg_ptr = local;
					LLVMTypeRef arg_type = LLVMGetElementType(LLVMTypeOf(arg_ptr));
					uint64_t mask = ((uint64_t) 1) << param_index;
					LLVMValueRef cond_pre = LLVMBuildAnd(builder, thunk_bitmap, LLVMConstInt(LLVMInt64Type(), mask, false), "");
					LLVMValueRef cond = LLVMBuildIsNotNull(builder, cond_pre, "");

					// Create the blocks
					LLVMBasicBlockRef thunk_unwrap = LLVMAppendBasicBlock(env->current_func, "thunk.unwrap");
					LLVMMoveBasicBlockAfter(thunk_unwrap, env->current_block);
					LLVMBasicBlockRef load_arg = LLVMAppendBasicBlock(env->current_func, "thunk.load");
					LLVMMoveBasicBlockAfter(load_arg, thunk_unwrap);
					LLVMBasicBlockRef post_thunk = LLVMAppendBasicBlock(env->current_func, "thunk.post");
					LLVMMoveBasicBlockAfter(post_thunk, load_arg);

					// Load the argument
					LLVMBuildCondBr(builder, cond, load_arg, thunk_unwrap);
					LLVMPositionBuilderAtEnd(builder, load_arg);
					LLVMValueRef loaded_arg = LLVMBuildLoad2(builder, arg_type, arg_ptr, "loaded");
					LLVMBuildBr(builder, post_thunk);

					// Unwrap the thunk
					LLVMPositionBuilderAtEnd(builder, thunk_unwrap);
					LLVMTypeRef func_type = LLVMFunctionType(arg_type, (LLVMTypeRef[]) {}, 0, false);
					LLVMValueRef thunk = LLVMBuildBitCast(builder, arg_ptr, LLVMPointerType(func_type, 0), "thunk");
					LLVMValueRef evaled = LLVMBuildCall2(builder, func_type, thunk, (LLVMValueRef[]) {}, 0, "evaled");
					LLVMBuildStore(builder, evaled, arg_ptr);
					LLVMBuildBr(builder, post_thunk);

					// Build phi instruction
					LLVMValueRef phi_vals[] = {loaded_arg, evaled};
					LLVMBasicBlockRef phi_blocks[] = {load_arg, thunk_unwrap};
					LLVMPositionBuilderAtEnd(builder, post_thunk);
					LLVMValueRef phi = LLVMBuildPhi(builder, arg_type, "");
					LLVMAddIncoming(phi, phi_vals, phi_blocks, 2);
					env->current_block = post_thunk;

					// Save the evaluated thunk for future usage
					set_llvm_param(env, ast->value.value, phi, 255);
					return phi;
				}

				// Get global
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
		env->local = push_llvm_scope(env->local);
		env->current_block = right_block;
		LLVMValueRef right = build_expression(ast->children[1], builder, env);
		right_block = env->current_block;
		env->local = pop_llvm_scope(env->local);

		// Build phi
		LLVMBuildBr(builder, post_and);
		LLVMPositionBuilderAtEnd(builder, post_and);
		env->current_block = post_and;
		LLVMValueRef phi = LLVMBuildPhi(builder, LLVMInt1Type(), "");
		LLVMValueRef incoming_values[] = {left, right};
		LLVMBasicBlockRef incoming_blocks[] = {from, right_block};
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
		env->local = push_llvm_scope(env->local);
		LLVMValueRef right = build_expression(ast->children[1], builder, env);
		right_block = env->current_block;
		env->local = pop_llvm_scope(env->local);

		// Build phi
		LLVMBuildBr(builder, post_or);
		LLVMPositionBuilderAtEnd(builder, post_or);
		env->current_block = post_or;
		LLVMValueRef phi = LLVMBuildPhi(builder, LLVMInt1Type(), "");
		LLVMValueRef incoming_values[] = {left, right};
		LLVMBasicBlockRef incoming_blocks[] = {from, right_block};
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
		env->local = push_llvm_scope(env->local);
		LLVMValueRef then_val = build_expression(ast->children[1], builder, env);
		then_block = env->current_block;
		env->local = pop_llvm_scope(env->local);

		// Build the branch and else body
		LLVMBuildBr(builder, post_if);
		LLVMPositionBuilderAtEnd(builder, else_block);
		env->current_block = else_block;
		env->local = push_llvm_scope(env->local);
		LLVMValueRef else_val = build_expression(ast->children[2], builder, env);
		else_block = env->current_block;
		env->local = pop_llvm_scope(env->local);

		// Build phi
		LLVMBuildBr(builder, post_if);
		LLVMPositionBuilderAtEnd(builder, post_if);
		env->current_block = post_if;
		LLVMValueRef phi = LLVMBuildPhi(builder, LLVMTypeOf(then_val), "");
		LLVMValueRef incoming_values[] = {then_val, else_val};
		LLVMBasicBlockRef incoming_blocks[] = {then_block, else_block};
		LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);
		return phi;
	} else return NULL;
}

// llvm_save_value(llvm_codegen_env_t*, char*, LLVMValueRef, LLVMBuilderRef) -> LLVMValueRef
// Saves a value as a global value, or a local if a local scope is present.
LLVMValueRef llvm_save_value(llvm_codegen_env_t* env, char* name, LLVMValueRef value, LLVMBuilderRef builder)
{
	// Set the global variable
	if (env->local == NULL)
	{
		// Get expression and global
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
		map_add(env->local->variables, name, value);
		return value;
	}
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
		LLVMValueRef value = build_expression(ast->children[1], builder, env);
		llvm_save_value(env, name, value, builder);

	// Functions
	} else if (ast->children[0]->value.type == LEX_TYPE_SYMBOL && ast->children[0]->children_count > 0)
	{
		// Find all closed locals
		name = ast->children[0]->value.value;
		hashmap_t* closed_locals = init_hashmap();
		if (env->local != NULL)
		{
			find_llvm_closure_locals(env, ast->children[1], closed_locals);
		}
		size_t closed_count = 0;
		char** closed_keys = map_keys(closed_locals, &closed_count, NULL);

		// Create the arguments
		size_t param_types_count = ast->children[0]->children_count + closed_count + 1;
		LLVMTypeRef param_types[param_types_count];
		param_types[0] = LLVMPointerType(LLVMInt64Type(), 0);
		for (size_t i = 1; i < closed_count + 1; i++)
		{
			param_types[i] = LLVMTypeOf(lookup_llvm_local(env, closed_keys[i - 1]));
		}
		for (size_t i = 1 + closed_count; i < param_types_count; i++)
		{
			param_types[i] = internal_type_to_llvm(env, ast->children[0]->children[i - 1 - closed_count]);
		}

		// Create the function
		char* func_name = malloc(strlen(ast->children[0]->value.value) + 5 + 1);
		strcpy(func_name, ast->children[0]->value.value);
		strcat(func_name, ".func");
		LLVMTypeRef func_type = LLVMFunctionType(internal_type_to_llvm(env, ast->children[1]), param_types, param_types_count, false);
		LLVMValueRef func = LLVMAddFunction(env->header_mod, func_name, func_type);
		free(func_name);

		// Save current state
		LLVMValueRef last_func = env->current_func;
		LLVMBasicBlockRef last_block = env->current_block;
		env->current_func = func;
		env->current_block = LLVMAppendBasicBlock(env->current_func, "entry");
		LLVMPositionBuilderAtEnd(builder, env->current_block);
		env->local = push_llvm_scope(env->local);

		// Set name of bitmap
		LLVMValueRef bitmap = LLVMGetParam(func, 0);
		LLVMSetValueName2(bitmap, "thunk.bitmap", 12);

		// Save closed over locals in a new scope
		for (size_t i = 1; i <= closed_count; i++)
		{
			LLVMValueRef arg = LLVMGetParam(func, i);
			char* arg_name = closed_keys[i - 1];
			LLVMSetValueName2(arg, arg_name, strlen(arg_name));

			if (lookup_llvm_param(env, arg_name) >= 64)
			{
				LLVMValueRef arg_alloca = LLVMBuildAlloca(builder, LLVMTypeOf(arg), arg_name);
				LLVMBuildStore(builder, arg, arg_alloca);
				set_llvm_param(env, arg_name, arg_alloca, i - 1);
			} else set_llvm_param(env, arg_name, arg, i - 1);
		}

		// Save parameters in the new scope
		for (size_t i = 1 + closed_count; i < param_types_count; i++)
		{
			LLVMValueRef arg = LLVMGetParam(func, i);
			char* arg_name = ast->children[0]->children[i - 1 - closed_count]->children[0]->value.value;
			LLVMSetValueName2(arg, arg_name, strlen(arg_name));

			LLVMValueRef arg_alloca = LLVMBuildAlloca(builder, LLVMTypeOf(arg), arg_name);
			LLVMBuildStore(builder, arg, arg_alloca);
			set_llvm_param(env, arg_name, arg_alloca, i - 1);
		}

		// Create the function
		LLVMValueRef ret = build_expression(ast->children[1], builder, env);

		// Return from function
		LLVMBuildRet(builder, ret);
		env->current_func = last_func;
		env->current_block = last_block;
		LLVMPositionBuilderAtEnd(builder, env->current_block);
		env->local = pop_llvm_scope(env->local);

		// Build a function application structure
		LLVMTypeRef func_app_type = LLVMGetTypeByName(env->header_mod, "func.app.type");
		LLVMValueRef func_alloca = LLVMBuildAlloca(builder, func_app_type, "");
		LLVMValueRef ref_count = LLVMBuildStructGEP(builder, func_alloca, 0, ".ref.count");
		LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), 1, false), ref_count);
		LLVMValueRef func_ptr = LLVMBuildStructGEP(builder, func_alloca, 1, ".func");
		LLVMBuildStore(builder, func, func_ptr);
		LLVMValueRef count_ptr = LLVMBuildStructGEP(builder, func_alloca, 2, ".arg.count");
		LLVMBuildStore(builder, LLVMConstInt(LLVMInt8Type(), closed_count, false), count_ptr);
		LLVMValueRef arity_ptr = LLVMBuildStructGEP(builder, func_alloca, 3, ".arity");
		LLVMBuildStore(builder, LLVMConstInt(LLVMInt8Type(), param_types_count - 1, false), arity_ptr);
		LLVMValueRef thunk_bitmap_ptr = LLVMBuildStructGEP(builder, func_alloca, 4, ".thunk.bitmap");
		LLVMBuildStore(builder, LLVMConstInt(LLVMInt64Type(), 0, false), thunk_bitmap_ptr);
		LLVMValueRef args_ptr_ptr = LLVMBuildStructGEP(builder, func_alloca, 5, ".args");

		// Create the list of arguments if necessary
		if (closed_count > 0)
		{
			LLVMValueRef args_malloc = LLVMBuildArrayMalloc(builder, LLVMInt64Type(), LLVMConstInt(LLVMInt32Type(), 64, false), ".args.malloc");
			LLVMBuildStore(builder, args_malloc, args_ptr_ptr);
			LLVMValueRef args_ptr = LLVMBuildLoad2(builder, LLVMPointerType(LLVMInt64Type(), 0), args_ptr_ptr, "");

			// Save each argument
			for (size_t i = 1; i <= closed_count; i++)
			{
				char* closed_name = closed_keys[i - 1];
				LLVMValueRef closed = lookup_llvm_local(env, closed_name);
				LLVMValueRef gep = LLVMBuildGEP2(builder, LLVMInt64Type(), args_ptr, (LLVMValueRef[]) {LLVMConstInt(LLVMInt64Type(), i - 1, false)}, 1, "");
				LLVMBuildStore(builder, closed, gep);
			}
		} else LLVMBuildStore(builder, LLVMConstInt(LLVMInt64Type(), 0, false), args_ptr_ptr);

		// Save the function application
		LLVMValueRef func_val = LLVMBuildLoad2(builder, func_app_type, func_alloca, "");
		llvm_save_value(env, ast->children[0]->value.value, func_val, builder);
		del_hashmap(closed_locals);
		return func_val;
	}

	return NULL;
}

// generate_code(ast_t*, llvm_codegen_env_t*) -> llvm_codegen_env_t*
// Generates llvm ir code from an ast.
llvm_codegen_env_t* generate_code(ast_t* ast, llvm_codegen_env_t* env)
{
	LLVMContextRef context;
	bool repl_mode = env != NULL;
	if (!repl_mode)
	{
		// Create the main module
		context = LLVMContextCreate();
		LLVMModuleRef main_mod = LLVMModuleCreateWithNameInContext("file", context);
		env = create_llvm_codegen_environment(main_mod);
		env->body_mod = main_mod;
	} else
	{
		// Create the executed module for repl
		context = LLVMGetModuleContext(env->header_mod);
		env->body_mod = LLVMModuleCreateWithNameInContext("stdin", context);

		// Create the repl variable
		LLVMValueRef repl_last = LLVMGetNamedGlobal(env->header_mod, "repl.last");
		if (repl_last != NULL) LLVMDeleteGlobal(repl_last);
		LLVMTypeRef repl_last_type = internal_type_to_llvm(env, ast->children[ast->children_count - 1]);
		repl_last = LLVMAddGlobal(env->header_mod, repl_last_type, "repl.last");
		LLVMSetLinkage(repl_last, LLVMExternalWeakLinkage);
	}

	// Create the main function
	LLVMTypeRef main_type = LLVMFunctionType(LLVMVoidType(), (LLVMTypeRef[]) {}, 0, false);
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

	// If in repl mode, save the last value
	if (repl_mode)
		LLVMBuildStore(builder, value, LLVMGetNamedGlobal(env->header_mod, "repl.last"));

	// Create a return instruction
	LLVMBuildRetVoid(builder);
	LLVMDisposeBuilder(builder);
	return env;
}

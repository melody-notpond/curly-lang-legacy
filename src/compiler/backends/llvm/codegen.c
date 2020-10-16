//
// Curly
// codegen.c: Implements code generation to LLVM IR.
//
// Created by jenra.
// Created on September 7 2020.
//

#include <stdio.h>
#include <string.h>

#include "../../../utils/list.h"
#include "codegen.h"
#include "functions.h"
#include "llvm_types.h"

// build_expression(ir_sexpr_t*, LLVMBuilderRef, llvm_codegen_env_t*) -> LLVMValueRef
// Builds an expression to LLVM IR.
LLVMValueRef build_expression(ir_sexpr_t* sexpr, LLVMBuilderRef builder, llvm_codegen_env_t* env);

// build_assignment(ir_sexpr_t*, LLVMBuilderRef, llvm_codegen_env_t*) -> LLVMValueRef
// Builds an assignment to LLVM IR.
LLVMValueRef build_assignment(ir_sexpr_t* sexpr, LLVMBuilderRef builder, llvm_codegen_env_t* env);

// build_infix(ir_sexpr_t*, LLVMBuilderRef, llvm_codegen_env_t*) -> LLVMValueRef
// Builds an infix expression to LLVM IR.
LLVMValueRef build_infix(ir_sexpr_t* sexpr, LLVMBuilderRef builder, llvm_codegen_env_t* env)
{
	// Build the operands
	LLVMValueRef left = build_expression(sexpr->infix.left, builder, env);
	LLVMValueRef right = build_expression(sexpr->infix.right, builder, env);

	// Cast ints to floats if necessary
	type_t* ltype = sexpr->infix.left->type;
	type_t* rtype = sexpr->infix.right->type;
	if (ltype->type_type == IR_TYPES_PRIMITIVE && rtype->type_type == IR_TYPES_PRIMITIVE)
	{
		if (!strcmp(ltype->type_name, "Int") && !strcmp(rtype->type_name, "Float"))
			left = LLVMBuildSIToFP(builder, left, LLVMDoubleType(), "");
		else if (!strcmp(ltype->type_name, "Float") && !strcmp(rtype->type_name, "Int"))
			right = LLVMBuildSIToFP(builder, right, LLVMDoubleType(), "");
	}

	// Build the operator
	switch(sexpr->infix.op)
	{
		case IR_BINOPS_MUL:
			if (sexpr->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(sexpr->type->type_name, "Int"))
				return LLVMBuildMul(builder, left, right, "");
			else return LLVMBuildFMul(builder, left, right, "");
		case IR_BINOPS_DIV:
			if (sexpr->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(sexpr->type->type_name, "Int"))
				return LLVMBuildSDiv(builder, left, right, "");
			else return LLVMBuildFDiv(builder, left, right, "");
		case IR_BINOPS_MOD:
			return LLVMBuildSRem(builder, left, right, "");
		case IR_BINOPS_ADD:
			if (sexpr->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(sexpr->type->type_name, "Int"))
				return LLVMBuildAdd(builder, left, right, "");
			else return LLVMBuildFAdd(builder, left, right, "");
		case IR_BINOPS_SUB:
			if (sexpr->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(sexpr->type->type_name, "Int"))
				return LLVMBuildSub(builder, left, right, "");
			else return LLVMBuildFSub(builder, left, right, "");
		case IR_BINOPS_BSL:
			return LLVMBuildShl(builder, left, right, "");
		case IR_BINOPS_BSR:
			return LLVMBuildLShr(builder, left, right, "");
		case IR_BINOPS_BITAND:
			return LLVMBuildAnd(builder, left, right, "");
		case IR_BINOPS_BITOR:
			return LLVMBuildOr(builder, left, right, "");
		case IR_BINOPS_BITXOR:
		case IR_BINOPS_BOOLXOR:
			return LLVMBuildXor(builder, left, right, "");
		case IR_BINOPS_CMPLT:
			if (ltype->type_type == IR_TYPES_PRIMITIVE && !strcmp(ltype->type_name, "Int")
			 && rtype->type_type == IR_TYPES_PRIMITIVE && !strcmp(rtype->type_name, "Int"))
				return LLVMBuildICmp(builder, LLVMIntSLT, left, right, "");
			else return LLVMBuildFCmp(builder, LLVMRealOLT, left, right, "");
		case IR_BINOPS_CMPLTE:
			if (ltype->type_type == IR_TYPES_PRIMITIVE && !strcmp(ltype->type_name, "Int")
			 && rtype->type_type == IR_TYPES_PRIMITIVE && !strcmp(rtype->type_name, "Int"))
				return LLVMBuildICmp(builder, LLVMIntSLE, left, right, "");
			else return LLVMBuildFCmp(builder, LLVMRealOLE, left, right, "");
		case IR_BINOPS_CMPGT:
			if (ltype->type_type == IR_TYPES_PRIMITIVE && !strcmp(ltype->type_name, "Int")
			 && rtype->type_type == IR_TYPES_PRIMITIVE && !strcmp(rtype->type_name, "Int"))
				return LLVMBuildICmp(builder, LLVMIntSGT, left, right, "");
			else return LLVMBuildFCmp(builder, LLVMRealOGT, left, right, "");
		case IR_BINOPS_CMPGTE:
			if (ltype->type_type == IR_TYPES_PRIMITIVE && !strcmp(ltype->type_name, "Int")
			 && rtype->type_type == IR_TYPES_PRIMITIVE && !strcmp(rtype->type_name, "Int"))
				return LLVMBuildICmp(builder, LLVMIntSGE, left, right, "");
			else return LLVMBuildFCmp(builder, LLVMRealOGE, left, right, "");
		case IR_BINOPS_CMPEQ:
			if (ltype->type_type == IR_TYPES_PRIMITIVE && !strcmp(ltype->type_name, "Int")
			 && rtype->type_type == IR_TYPES_PRIMITIVE && !strcmp(rtype->type_name, "Int"))
				return LLVMBuildICmp(builder, LLVMIntEQ, left, right, "");
			else return LLVMBuildFCmp(builder, LLVMRealOEQ, left, right, "");
		case IR_BINOPS_CMPNEQ:
			if (ltype->type_type == IR_TYPES_PRIMITIVE && !strcmp(ltype->type_name, "Int")
			 && rtype->type_type == IR_TYPES_PRIMITIVE && !strcmp(rtype->type_name, "Int"))
				return LLVMBuildICmp(builder, LLVMIntNE, left, right, "");
			else return LLVMBuildFCmp(builder, LLVMRealONE, left, right, "");
		default:
			return NULL;
	}
}

// build_expression(ir_sexpr_t*, LLVMBuilderRef, llvm_codegen_env_t*) -> LLVMValueRef
// Builds an expression to LLVM IR.
LLVMValueRef build_expression(ir_sexpr_t* sexpr, LLVMBuilderRef builder, llvm_codegen_env_t* env)
{
	switch (sexpr->tag)
	{
		case CURLY_IR_TAGS_INT:
			return LLVMConstInt(LLVMInt64Type(), sexpr->i64, false);
		case CURLY_IR_TAGS_FLOAT:
			return LLVMConstReal(LLVMDoubleType(), sexpr->f64);
		case CURLY_IR_TAGS_BOOL:
			return LLVMConstInt(LLVMInt1Type(), sexpr->i1, false);
		case CURLY_IR_TAGS_SYMBOL:
		{
			// Get local
			LLVMValueRef local = lookup_llvm_local(env, sexpr->symbol);
			if (local != NULL)
			// {
				// // Local is definitely not a thunk
				// uint64_t param_index = lookup_llvm_param(env, sexpr->symbol);
				// if (param_index >= 64)
					return local;

			// 	// Local is a parameter that has not been checked for thunkiness
			// 	LLVMValueRef thunk_bitmap_ptr = LLVMGetParam(env->current_func, 0);
			// 	LLVMValueRef thunk_bitmap = LLVMBuildLoad2(builder, LLVMInt64Type(), thunk_bitmap_ptr, "thunk.bitmap.deref");
			// 	LLVMValueRef arg_ptr = local;
			// 	LLVMTypeRef arg_type = LLVMGetElementType(LLVMTypeOf(arg_ptr));
			// 	uint64_t mask = ((uint64_t) 1) << param_index;
			// 	LLVMValueRef cond_pre = LLVMBuildAnd(builder, thunk_bitmap, LLVMConstInt(LLVMInt64Type(), mask, false), "");
			// 	LLVMValueRef cond = LLVMBuildIsNotNull(builder, cond_pre, "");

			// 	// Create the blocks
			// 	LLVMBasicBlockRef thunk_unwrap = LLVMAppendBasicBlock(env->current_func, "thunk.unwrap");
			// 	LLVMMoveBasicBlockAfter(thunk_unwrap, env->current_block);
			// 	LLVMBasicBlockRef load_arg = LLVMAppendBasicBlock(env->current_func, "thunk.load");
			// 	LLVMMoveBasicBlockAfter(load_arg, thunk_unwrap);
			// 	LLVMBasicBlockRef post_thunk = LLVMAppendBasicBlock(env->current_func, "thunk.post");
			// 	LLVMMoveBasicBlockAfter(post_thunk, load_arg);

			// 	// Load the argument
			// 	LLVMBuildCondBr(builder, cond, load_arg, thunk_unwrap);
			// 	LLVMPositionBuilderAtEnd(builder, load_arg);
			// 	LLVMValueRef loaded_arg = LLVMBuildLoad2(builder, arg_type, arg_ptr, "loaded");
			// 	LLVMBuildBr(builder, post_thunk);

			// 	// Unwrap the thunk
			// 	LLVMPositionBuilderAtEnd(builder, thunk_unwrap);
			// 	LLVMTypeRef func_type = LLVMFunctionType(arg_type, (LLVMTypeRef[]) {}, 0, false);
			// 	LLVMValueRef thunk = LLVMBuildBitCast(builder, arg_ptr, LLVMPointerType(func_type, 0), "thunk");
			// 	LLVMValueRef evaled = LLVMBuildCall2(builder, func_type, thunk, (LLVMValueRef[]) {}, 0, "evaled");
			// 	LLVMBuildStore(builder, evaled, arg_ptr);
			// 	LLVMBuildBr(builder, post_thunk);

			// 	// Build phi instruction
			// 	LLVMValueRef phi_vals[] = {loaded_arg, evaled};
			// 	LLVMBasicBlockRef phi_blocks[] = {load_arg, thunk_unwrap};
			// 	LLVMPositionBuilderAtEnd(builder, post_thunk);
			// 	LLVMValueRef phi = LLVMBuildPhi(builder, arg_type, "");
			// 	LLVMAddIncoming(phi, phi_vals, phi_blocks, 2);
			// 	env->current_block = post_thunk;

			// 	// Save the evaluated thunk for future usage
			// 	set_llvm_param(env, sexpr->symbol, phi, 255);
			// 	return phi;
			// }

			// Get global
			LLVMValueRef global = LLVMGetNamedGlobal(env->header_mod, sexpr->symbol);
			return LLVMBuildLoad(builder, global, "");
		}
		case CURLY_IR_TAGS_INFIX:
			switch (sexpr->infix.op)
			{
				case IR_BINOPS_BOOLAND:
				{
					// Build the left operand
					LLVMValueRef left = build_expression(sexpr->infix.left, builder, env);

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
					LLVMValueRef right = build_expression(sexpr->infix.right, builder, env);
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
				}

				case IR_BINOPS_BOOLOR:
				{
					// Build the left operand
					LLVMValueRef left = build_expression(sexpr->infix.left, builder, env);

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
					LLVMValueRef right = build_expression(sexpr->infix.right, builder, env);
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
				}

				default:
					return build_infix(sexpr, builder, env);
			}
		case CURLY_IR_TAGS_PREFIX:
			switch (sexpr->prefix.op)
			{
				case IR_BINOPS_NEG:
				{
					LLVMValueRef operand = build_expression(sexpr->prefix.operand, builder, env);
					if (sexpr->prefix.operand->type->type_type == IR_TYPES_PRIMITIVE && !strcmp(sexpr->prefix.operand->type->type_name, "Int"))
						return LLVMBuildNeg(builder, operand, "");
					else return LLVMBuildFNeg(builder, operand, "");
				}
				default:
					puts("Unsupported prefix operator!");
					return NULL;
			}
		case CURLY_IR_TAGS_LOCAL_SCOPE:
			// Push local scope
			env->local = push_llvm_scope(env->local);

			// Build all the assignments
			for (size_t i = 0; i < sexpr->local_scope.assign_count; i++)
			{
				if (sexpr->local_scope.assigns[i]->tag == CURLY_IR_TAGS_ASSIGN)
					build_assignment(sexpr->local_scope.assigns[i], builder, env);
			}

			// Build the scope
			LLVMValueRef value = build_expression(sexpr->local_scope.value, builder, env);

			// Pop local scope
			env->local = pop_llvm_scope(env->local);
			return value;
		case CURLY_IR_TAGS_IF:
		{
			// Build condition
			LLVMValueRef cond = build_expression(sexpr->if_expr.cond, builder, env);

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
			LLVMValueRef then_val = build_expression(sexpr->if_expr.then, builder, env);
			then_block = env->current_block;
			env->local = pop_llvm_scope(env->local);

			// Build the branch and else body
			LLVMBuildBr(builder, post_if);
			LLVMPositionBuilderAtEnd(builder, else_block);
			env->current_block = else_block;
			env->local = push_llvm_scope(env->local);
			LLVMValueRef else_val = build_expression(sexpr->if_expr.elsy, builder, env);
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
		}
		default:
			puts("Unsupported S expression!");
			return NULL;
	}
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

// build_assignment(ir_sexpr_t*, LLVMBuilderRef, llvm_codegen_env_t*) -> LLVMValueRef
// Builds an assignment to LLVM IR.
LLVMValueRef build_assignment(ir_sexpr_t* sexpr, LLVMBuilderRef builder, llvm_codegen_env_t* env)
{
	LLVMValueRef value = build_expression(sexpr->assign.value, builder, env);
	return llvm_save_value(env, sexpr->assign.name, value, builder);

	// // var = expr and var: type = expr
	// char* name = NULL;
	// if (ast->children[0]->value.type == LEX_TYPE_SYMBOL && ast->children[0]->children_count == 0)
	// 	name = ast->children[0]->value.value;
	// else if (ast->children[0]->value.type == LEX_TYPE_COLON)
	// 	name = ast->children[0]->children[0]->value.value;

	// // New variable (typed or untyped)
	// if (name != NULL)
	// {
	// 	LLVMValueRef value = build_expression(ast->children[1], builder, env);
	// 	llvm_save_value(env, name, value, builder);

	// // Functions
	// } else if (ast->children[0]->value.type == LEX_TYPE_SYMBOL && ast->children[0]->children_count > 0)
	// {
	// 	// Create the function
	// 	LLVMTypeRef func_app_type = LLVMGetTypeByName(env->header_mod, "func.app.type");
	// 	LLVMTypeRef func_app_ptr_type = LLVMPointerType(func_app_type, 0);
	// 	LLVMTypeRef func_type = LLVMFunctionType(func_app_ptr_type, (LLVMTypeRef[]) {func_app_ptr_type}, 1, false);
	// 	char* func_name = malloc(strlen(ast->children[0]->value.value) + 6);
	// 	func_name[0] = '\0';
	// 	strcat(func_name, ast->children[0]->value.value);
	// 	strcat(func_name, ".func");
	// 	LLVMValueRef func = LLVMAddFunction(env->header_mod, func_name, func_type);
	// 	free(func_name);

	// 	// Save state and move builder to the start of the function
	// 	LLVMBasicBlockRef last_block = env->current_block;
	// 	LLVMValueRef last_func = env->current_func;
	// 	LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
	// 	env->current_func = func;
	// 	env->current_block = entry;
	// 	LLVMPositionBuilderAtEnd(builder, entry);

	// 	// Check for argument count
	// 	LLVMValueRef app_arg = LLVMGetParam(func, 0);
	// 	LLVMSetValueName2(app_arg, ".app", 4);
	// 	LLVMValueRef argc_ptr = LLVMBuildStructGEP2(builder, func_app_type, app_arg, 2, "argc-ptr");
	// 	LLVMValueRef argc = LLVMBuildLoad2(builder, LLVMInt8Type(), argc_ptr, "argc");
	// 	uint8_t arity = ast->children[0]->children_count;
	// 	LLVMValueRef arity_val = LLVMConstInt(LLVMInt8Type(), arity, false);
	// 	LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntULT, argc, arity_val, "");

	// 	// Build branch
	// 	LLVMBasicBlockRef ret = LLVMAppendBasicBlock(func, "return");
	// 	LLVMBasicBlockRef cont = LLVMAppendBasicBlock(func, "continue");
	// 	LLVMBuildCondBr(builder, cond, ret, cont);

	// 	// Return from the function
	// 	LLVMPositionBuilderAtEnd(builder, ret);
	// 	LLVMBuildRet(builder, app_arg);

	// 	// Build arguments
	// 	LLVMPositionBuilderAtEnd(builder, cont);
	// 	LLVMValueRef args = LLVMBuildStructGEP2(builder, func_app_type, app_arg, 4, "args");
	// 	env->local = push_llvm_scope(env->local);
	// 	env->current_block = cont;
	// 	for (uint8_t i = 0; i < arity; i++)
	// 	{
	// 		LLVMValueRef arg = LLVMBuildGEP2(builder, func_app_type, args, (LLVMValueRef[]) {LLVMConstInt(LLVMInt8Type(), i, false)}, 1, "");
	// 		if (ast->children[0]->children[i]->type->type_type != IR_TYPES_FUNC)
	// 		{
	// 			arg = LLVMBuildStructGEP2(builder, func_app_type, arg, 0, "");
	// 			arg = LLVMBuildBitCast(builder, arg, LLVMPointerType(internal_type_to_llvm(env, ast->children[0]->children[i]), 0), "");
	// 		}
	// 		char* arg_name = ast->children[0]->children[i]->children[0]->value.value;
	// 		LLVMSetValueName2(arg, arg_name, strlen(arg_name));
	// 		set_llvm_param(env, arg_name, arg, i);
	// 	}

	// 	// Create the function
	// 	LLVMValueRef ret_val = build_expression(ast->children[1], builder, env);

	// 	// Return from function
	// 	LLVMBuildRet(builder, ret_val);
	// 	env->current_func = last_func;
	// 	env->current_block = last_block;
	// 	LLVMPositionBuilderAtEnd(builder, env->current_block);
	// 	env->local = pop_llvm_scope(env->local);

	// 	// Build a function application structure
	// 	LLVMValueRef func_alloca = LLVMBuildAlloca(builder, func_app_type, "");
	// 	LLVMValueRef ref_count = LLVMBuildStructGEP(builder, func_alloca, 0, ".ref.count");
	// 	LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), 1, false), ref_count);
	// 	LLVMValueRef func_ptr = LLVMBuildStructGEP(builder, func_alloca, 1, ".func");
	// 	LLVMBuildStore(builder, func, func_ptr);
	// 	LLVMValueRef count_ptr = LLVMBuildStructGEP(builder, func_alloca, 2, ".arg.count");
	// 	LLVMBuildStore(builder, LLVMConstInt(LLVMInt8Type(), 0, false), count_ptr);
	// 	LLVMValueRef thunk_bitmap_ptr = LLVMBuildStructGEP(builder, func_alloca, 3, ".thunk.bitmap");
	// 	LLVMBuildStore(builder, LLVMConstInt(LLVMInt64Type(), 0, false), thunk_bitmap_ptr);
	// 	LLVMValueRef args_ptr_ptr = LLVMBuildStructGEP(builder, func_alloca, 4, ".args");
	// 	LLVMBuildStore(builder, LLVMConstInt(LLVMInt64Type(), 0, false), args_ptr_ptr);

	// 	// Save the function application
	// 	LLVMValueRef func_val = LLVMBuildLoad2(builder, func_app_type, func_alloca, "");
	// 	llvm_save_value(env, ast->children[0]->value.value, func_val, builder);
	// 	// del_hashmap(closed_locals);
	// 	return func_val;
	// }

	// return NULL;
}

// generate_code(curly_ir_t, llvm_codegen_env_t*) -> llvm_codegen_env_t*
// Generates llvm ir code from an ast.
llvm_codegen_env_t* generate_code(curly_ir_t ir, llvm_codegen_env_t* env)
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
		LLVMTypeRef repl_last_type = internal_type_to_llvm(env, ir.expr[ir.expr_count - 1]->type);
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
	for (size_t i = 0; i < ir.expr_count; i++)
	{
		if (ir.expr[i]->tag == CURLY_IR_TAGS_ASSIGN)
			value = build_assignment(ir.expr[i], builder, env);
		else if (ir.expr[i]->tag != CURLY_IR_TAGS_DECLARE)
			value = build_expression(ir.expr[i], builder, env);
	}

	// If in repl mode, save the last value
	if (repl_mode)
		LLVMBuildStore(builder, value, LLVMGetNamedGlobal(env->header_mod, "repl.last"));

	// Create a return instruction
	LLVMBuildRetVoid(builder);
	LLVMDisposeBuilder(builder);
	return env;
}

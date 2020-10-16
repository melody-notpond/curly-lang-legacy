// 
// ir
// generate_ir.h: Header file for generate_ir.c.
// 
// Created by jenra.
// Created on October 12 2020.
// 

#ifndef GENERATE_IR_H
#define GENERATE_IR_H

#include "../correctness/types.h"
#include "../parse/ast.h"

// Represents the tag of an IR S expression.
typedef enum
{
	CURLY_IR_TAGS_INT,
	CURLY_IR_TAGS_FLOAT,
	CURLY_IR_TAGS_BOOL,
	CURLY_IR_TAGS_SYMBOL,
	CURLY_IR_TAGS_INFIX,
	CURLY_IR_TAGS_PREFIX,
	CURLY_IR_TAGS_ASSIGN,
	CURLY_IR_TAGS_DECLARE,
	CURLY_IR_TAGS_LOCAL_SCOPE,
	CURLY_IR_TAGS_IF
} ir_types_t;

// Represents an infix operation.
typedef enum
{
	IR_BINOPS_NEG,
	IR_BINOPS_MUL,
	IR_BINOPS_DIV,
	IR_BINOPS_MOD,
	IR_BINOPS_ADD,
	IR_BINOPS_SUB,
	IR_BINOPS_BSL,
	IR_BINOPS_BSR,
	IR_BINOPS_BITAND,
	IR_BINOPS_BITOR,
	IR_BINOPS_BITXOR,
	IR_BINOPS_CMPEQ,
	IR_BINOPS_CMPNEQ,
	IR_BINOPS_CMPLT,
	IR_BINOPS_CMPLTE,
	IR_BINOPS_CMPGT,
	IR_BINOPS_CMPGTE,
	IR_BINOPS_CMPIN,
	IR_BINOPS_BOOLAND,
	IR_BINOPS_BOOLOR,
	IR_BINOPS_BOOLXOR,
	IR_BINOPS_SPAN
} ir_binops_t;

// An S expression in IR code.
typedef struct s_ir_sexpr
{
	// The type of the expression.
	type_t* type;

	// The position in the string the expression was found at.
	size_t pos;
	int lino;
	int charpos;
	// The tag for the tagged union.
	ir_types_t tag;

	// The value of the expression.
	union
	{
		int64_t i64;
		double f64;
		bool i1;
		char* symbol;

		// Infix expressions.
		struct
		{
			ir_binops_t op;
			struct s_ir_sexpr* left;
			struct s_ir_sexpr* right;
		} infix;

		// Prefix expressions.
		struct
		{
			ir_binops_t op;
			struct s_ir_sexpr* operand;
		} prefix;

		// Assignments.
		struct
		{
			char* name;
			struct s_ir_sexpr* value;
		} assign;

		// Declarations.
		struct
		{
			char* name;
		} declare;

		// Local scopes.
		struct
		{
			struct s_ir_sexpr** assigns;
			size_t assign_count;

			struct s_ir_sexpr* value;
		} local_scope;

		// If expressions.
		struct
		{
			struct s_ir_sexpr* cond;
			struct s_ir_sexpr* then;
			struct s_ir_sexpr* elsy;
		} if_expr;
	};
} ir_sexpr_t;

// Represents the IR.
typedef struct
{
	// The list of all expressions.
	ir_sexpr_t** expr;
	size_t expr_count;
} curly_ir_t;

// convert_ast_to_ir(ast_t*, ir_scope_t*) -> curly_ir_t
// Converts a given ast root to IR.
curly_ir_t convert_ast_to_ir(ast_t* ast, ir_scope_t* scope);

// print_ir(curly_ir_t) -> void
// Prints out IR to stdout.
void print_ir(curly_ir_t ir);

// clean_ir(curly_ir_t) -> void
// Cleans up Curly IR.
void clean_ir(curly_ir_t ir);

#endif /* GENERATE_IR_H */

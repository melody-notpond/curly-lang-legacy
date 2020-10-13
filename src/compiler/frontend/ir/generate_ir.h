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

// Represents the type of an ir line
typedef enum
{
	CURLY_IR_TYPES_INT,
	CURLY_IR_TYPES_DOUBLE,
	CURLY_IR_TYPES_SYMBOL,
	CURLY_IR_TYPES_INFIX,
	CURLY_IR_TYPES_ASSIGN,
	CURLY_IR_TYPES_DECLARE,
	CURLY_IR_TYPES_LOCAL_SCOPE
} ir_types_t;

// Represents an infix operation
typedef enum
{
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
	IR_BINOPS_NEG,
	IR_BINOPS_SPAN
} ir_binops_t;

// A piece of ir code
typedef struct s_ir_expr
{
	// The type of the ir
	ir_types_t type;

	// The position in the string the expression was found at
	size_t pos;
	int lino;
	int charpos;

	// The value of the expression
	union
	{
		int i64;
		double f64;
		char* symbol;

		// Infix expressions
		struct
		{
			ir_binops_t op;
			struct s_ir_expr* left;
			struct s_ir_expr* right;
		} infix;

		// Prefix expressions
		struct
		{
			ir_binops_t op;
			struct s_ir_expr* operand;
		} prefix;

		// Assignments
		struct
		{
			char* name;
			struct s_ir_expr* value;
		} assign;

		// Declarations
		struct
		{
			char* name;
		} declare;

		// Local scopes
		struct
		{
			struct s_ir_expr** assigns;
			size_t assign_count;

			struct s_ir_expr* value;
		} local_scope;

		// If expressions
		struct
		{
			struct s_ir_expr* cond;
			struct s_ir_expr* then;
			struct s_ir_expr* elsy;
		} if_expr;
	};
} ir_expr_t;

// Represents the ir.
typedef struct
{
	// The list of all expressions
	ir_expr_t** expr;
	size_t expr_count;
} curly_ir_t;

#endif /* GENERATE_IR_H */

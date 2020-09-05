// 
// correctness
// type_generators.h: Header file for type_generators.c.
// 
// Created by jenra.
// Created on September 5 2020.
// 

#ifndef TYPE_GENERATORS_H
#define TYPE_GENERATORS_H

#include "scope.h"
#include "types.h"

// generate_type(ast_t*, ir_scope_t*, ast_t*, type_t*) -> type_t*
// Generates a type from an infix expression.
type_t* generate_type(ast_t* ast, ir_scope_t* scope, ast_t* self, type_t* head);

// generate_enum(ast_t*, ir_scope_t*, type_t*) -> type_t*
// Generates an enum type from a given ast.
type_t* generate_enum(ast_t* ast, ir_scope_t* scope, type_t* head);

#endif /* TYPE_GENERATORS_H */

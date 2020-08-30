//
// correctness
// check.h: Header file for check.c.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#ifndef check_h
#define check_h

#include "../parse/ast.h"
#include "scope.h"

// check_correctness(ast_t*) -> void
// Checks the correctness of an ast.
bool check_correctness(ast_t* ast);

#endif /* check_h */

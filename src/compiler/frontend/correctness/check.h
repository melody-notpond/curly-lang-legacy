//
// correctness
// check.h: Header file for check.c.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#ifndef CHECK_H
#define CHECK_H

#include "../parse/ast.h"
#include "scope.h"

// check_correctness(ast_t*) -> void
// Checks the correctness of an ast.
bool check_correctness(ast_t* ast);

#endif /* CHECK_H */

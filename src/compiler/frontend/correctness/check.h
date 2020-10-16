//
// correctness
// check.h: Header file for check.c.
// 
// Created by jenra.
// Created on August 29 2020.
// 

#ifndef CHECK_H
#define CHECK_H

#include "../ir/generate_ir.h"
#include "scope.h"

// check_correctness(curly_ir_t, ir_scope_t*) -> void
// Checks the correctness of IR.
bool check_correctness(curly_ir_t ir, ir_scope_t* scope);

#endif /* CHECK_H */

//
// correctness
// check_assign.h: Header file for check_assign.c.
// 
// Created by jenra.
// Created on September 5 2020.
// 

#ifndef CHECK_ASSIGN_H
#define CHECK_ASSIGN_H

#include <stdlib.h>

#include "scope.h"
#include "types.h"

// check_assign(ast_t*, ir_scope_t*, bool, bool) -> bool
// Checks if an assignment is valid.
bool check_assign(ast_t* ast, ir_scope_t* scope, bool get_real_type, bool disable_new_vars);

#endif /* CHECK_ASSIGN_H */

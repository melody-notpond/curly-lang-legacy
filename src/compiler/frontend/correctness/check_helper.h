// 
// correctness
// check_helper.h: Header file for check.c.
// 
// Created by jenra.
// Created on September 5 2020.
// 

#ifndef CHECK_HELPER_H
#define CHECK_HELPER_H

#include "scope.h"
#include "types.h"

// check_correctness_helper(ast_t*, ir_scope_t*, bool, bool) -> void
// Helper function for check_correctness.
bool check_correctness_helper(ast_t* ast, ir_scope_t* scope, bool get_real_type, bool disable_new_vars);

#endif /* CHECK_HELPER_H */

// 
// parse
// parser.h: Header file for parser.c.
// 
// Created by jenra.
// Created on August 28 2020.
// 

#ifndef parser_h
#define parser_h

#include "ast.h"
#include "lexer.h"

// statement: assignment | expression
parse_result_t statement(lexer_t* lex);

#endif /* parser_h */

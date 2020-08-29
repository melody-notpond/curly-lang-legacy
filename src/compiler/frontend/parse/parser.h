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

// lang_parser(lexer_t*) -> parse_result_t
// Parses the curly language.
// lang_parser: statement*
parse_result_t lang_parser(lexer_t* lex);

#endif /* parser_h */

// 
// parse
// parser.h: Header file for parser.c.
// 
// Created by jenra.
// Created on August 28 2020.
// 

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

// lang_parser(lexer_t*) -> parse_result_t
// Parses the curly language.
// lang_parser: statement*
parse_result_t lang_parser(lexer_t* lex);

#endif /* PARSER_H */

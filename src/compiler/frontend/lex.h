//
// Curly
// lex.h: Header file for lex.c.
//
// jenra
// March 1 2020
//

#ifndef lex_h
#define lex_h

#include "lexer.h"

// curly_lexer_func(lexer_t*) -> lexeme_t*
// The lexer function used for lexing curly programs.
lexeme_t* curly_lexer_func(lexer_t* lex);

#endif /* lex_h */

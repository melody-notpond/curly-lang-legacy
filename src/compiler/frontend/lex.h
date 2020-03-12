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

// Represents the type of the lexeme.
enum
{
	LEX_TYPE_INVALID = -1,
	LEX_TYPE_EOF = 0,

	LEX_TYPE_INT,
	LEX_TYPE_DECIMAL,
	LEX_TYPE_CHAR,
	LEX_TYPE_SYMBOL,
	LEX_TYPE_PRIMATIVE,

	LEX_TYPE_KEYWORD,
	LEX_TYPE_POSTFIX,
	LEX_TYPE_INFIX_LEVEL_MUL,
	LEX_TYPE_INFIX_LEVEL_ADD,
	LEX_TYPE_INFIX_LEVEL_COMPARE,
	LEX_TYPE_SPECIAL_CHAR,
	LEX_TYPE_GROUPING,
};

// curly_lexer_func(lexer_t*) -> lexeme_t
// The lexer function used for lexing curly programs.
lexeme_t curly_lexer_func(lexer_t* lex);

#endif /* lex_h */

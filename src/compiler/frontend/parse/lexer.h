// 
// parse
// lexer.h: Header file for lexer.c.
// 
// Created by jenra.
// Created on July 27 2020.
// 

#ifndef lexer_h
#define lexer_h

#include <stdlib.h>

typedef enum
{
	LEX_TAG_NONE,
	LEX_TAG_OPERAND,
	LEX_TAG_OPERATOR,
	LEX_TAG_INFIX_OPERATOR
} lex_tag_t;

typedef enum
{
	LEX_TYPE_NONE,
	LEX_TYPE_EOF,
	LEX_TYPE_INT,
	LEX_TYPE_FLOAT,
	LEX_TYPE_LGROUP,
	LEX_TYPE_RGROUP,
	LEX_TYPE_COLON,
	LEX_TYPE_NEWLINE,
	LEX_TYPE_COMMA,
	LEX_TYPE_SYMBOL,
	LEX_TYPE_KEYWORD,
	LEX_TYPE_BOOL,
	LEX_TYPE_NIL,
	LEX_TYPE_ASSIGN,
	LEX_TYPE_COMPARE,
	LEX_TYPE_DOT,
	LEX_TYPE_RANGE,
	LEX_TYPE_MULDIV,
	LEX_TYPE_ADDSUB,
	LEX_TYPE_BITSHIFT,
	LEX_TYPE_AND,
	LEX_TYPE_OR,
	LEX_TYPE_XOR,
	LEX_TYPE_AMP,
	LEX_TYPE_BAR,
	LEX_TYPE_CARET,
	LEX_TYPE_STRING
} lex_type_t;

// Represents a token.
typedef struct
{
	// The type of the token/
	lex_type_t type;

	// The tag of the token; ie, whether it's an operand, operator, grouping symbol, et cetera.
	lex_tag_t tag;

	// The value of the token.
	char* value;

	// The position the token was found.
	size_t pos;
	int lino;
	int charpos;
} token_t;

// Represents the current state of the lexer.
typedef struct
{
	// The string being parsed.
	char* string;

	// The current position of the lexer.
	size_t pos;
	int lino;
	int charpos;

	// The list of tokens already parsed.
	token_t* tokens;
	size_t count;
	size_t size;

	// The current position in the list of tokens.
	size_t token_pos;
} lexer_t;

// init_lexer(lexer_t*, char*) -> void
// Initialises a lexer.
void init_lexer(lexer_t* lex, char* string);

// lex_type_string(lex_type_t) -> char*
// Converts a lex_type_t into a string.
char* lex_type_string(lex_type_t type);

// lex_next(lexer_t*) -> token_t*
// Consumes the next token in the string.
token_t* lex_next(lexer_t* lex);

// cleanup_lexer(lexer_t*) -> void
// Frees memory associated with the lexer.
void cleanup_lexer(lexer_t* lex);

#endif /* lexer_h */

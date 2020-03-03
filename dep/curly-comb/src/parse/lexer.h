//
// Curly parser combinator
// lexer.h: Header file for lexer.c.
//
// jenra
// February 25 2020
//

#ifndef lexer_h
#define lexer_h

#include <stdlib.h>
#include <string.h>

// Represents a lexer.
typedef struct
{
	// The string the lexer is tokenising.
	char* string;

	// If true, will not return any whitespace characters
	bool ignore_whitespace;

	// The position of the lexer.
	size_t position;

	// The line the lexer is parsing.
	int line;

	// The position of the lexer relative to the line position.
	int char_pos;
} lexer_t;

// lex_str(char*, bool) -> lexer_t
// Initialises a new lexer with a given string.
lexer_t lex_str(char* string, bool ignore_whitespace);

// lex_skip_whitespace(lexer_t*) -> void
// Skips all whitespace, if applicable.
void lex_skip_whitespace(lexer_t* lex);

// lex_next(lexer_t*) -> lexeme_t
// Pops the next character.
char lex_next(lexer_t* lex);

// clean_lex(lexer_t*) -> void
// Cleans up memory used by a lexer.
void clean_lex(lexer_t* lex);

#endif /* lexer_h */

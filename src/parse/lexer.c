//
// Curly parser combinator
// lexer.c: Implements a lexer.
//
// jenra
// February 25 2020
//

#include <stdbool.h>

#include "lexer.h"

// lex_str(char*) -> lexer_t
// Initialises a new lexer with a given string.
lexer_t lex_str(char* string)
{
	lexer_t lex;
	lex.string = strdup(string);
	lex.position = 0;
	lex.line = 1;
	lex.char_pos = 0;
	return lex;
}

// lex_next(lexer_t*) -> lexeme_t
// Pops the next character.
char lex_next(lexer_t* lex)
{
	// Next character
	char c = lex->string[lex->position];

	if (c == '\n')
	{
		// New line updates metadata
		lex->char_pos = 0;
		lex->line++;
		lex->position++;
	} else if (c != '\0')
	{
		// Not end of string means more data can be popped
		lex->position++;
		lex->char_pos++;
	}

	return c;
}

// clean_lex(lexer_t*) -> void
// Cleans up memory used by a lexer.
void clean_lex(lexer_t* lex)
{
	free(lex->string);
	lex->string = NULL;
}

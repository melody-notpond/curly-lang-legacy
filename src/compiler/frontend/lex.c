//
// Curly
// lex.c: Creates the lexer function for curly.
//
// jenra
// March 11 2020
//

#include "lex.h"

// create_int(lexer_t*, lexeme_t*) -> void
// Creates an integer.
void create_int(lexer_t* lex, lexeme_t* token)
{
	token->type = LEX_TYPE_INT;

	// Save the current state
	lexer_t save;
	lex_save(lex, &save);

	while (true)
	{
		// Get the next character
		char c = lex_next_char(lex);

		// Still an integer
		if ('0' <= c && c <= '9')
			lex_save(lex, &save);
		else
		{
			// No longer an integer
			lex_revert(lex, &save);
			break;
		}
	}
}

// curly_lexer_func(lexer_t*) -> lexeme_t
// The lexer function used for lexing curly programs.
lexeme_t curly_lexer_func(lexer_t* lex)
{
	// Empty token
	lexeme_t token = init_lexeme(lex);

	// Save the current state
	lexer_t save;
	lex_save(lex, &save);

	// Determine what to do based on the character received
	char c = lex_next_char(lex);
	if ('0' <= c && c <= '9')
		create_int(lex, &token);
	else
	{
		// Invalid token
		lex_revert(lex, &save);
		return token;
	}

	// Copy the string into the token and return
	size_t length = lex->file_pos - token.position;
	lex_revert(lex, &save);
	token.string = lex_next_str(lex, length);
	return token;
}

//
// Curly
// lex.c: Creates the lexer function for curly.
//
// jenra
// March 11 2020
//

#include "lex.h"

// skip_whitespace_and_comments(lexer_t*) -> void
// Skips all whitespace and comments.
void skip_whitespace_and_comments(lexer_t* lex)
{
	lexer_t save;
	lex_save(lex, &save);

	bool in_comment = false;
	while (true)
	{
		char c = lex_next_char(lex);

		if (in_comment && c == '\n')
			in_comment = false;
		else if (!in_comment)
		{
			if (c == '#')
				in_comment = true;
			else if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v'))
				break;
		}

		lex_save(lex, &save);
	}

	lex_revert(lex, &save);
}

// create_decimal(lexer_t*, lexeme_t*, bool)
// Creates a decimal.
bool create_decimal(lexer_t* lex, lexeme_t* token, bool dot_mode)
{
	// Save
	lexer_t save;
	lex_save(lex, &save);

	// Following a decimal point
	if (dot_mode)
	{
		// Check that the next character is a digit
		char c = lex_next_char(lex);
		if (c < '0' || c > '9')
			return false;
		do
		{
			// Save and get the next character
			lex_save(lex, &save);
			c = lex_next_char(lex);

			// Check if it's scientific notation time
			if (c == 'e' || c == 'E')
				dot_mode = false;

		// Do this until the character is not a digit
		} while ('0' <= c && c <= '9');

		// Revert if not going to check scientific notation and set the type
		if (dot_mode)
			lex_revert(lex, &save);
		token->type = LEX_TYPE_DECIMAL;
	}

	// Scientific notation
	if (!dot_mode)
	{
		// Optional +/-
		char c = lex_next_char(lex);
		if (c == '+' || c == '-')
			c = lex_next_char(lex);

		// Check if character is a digit
		if (c < '0' || c > '9')
		{
			lex_revert(lex, &save);
			return token->type == LEX_TYPE_DECIMAL;
		}
		do
		{
			// Save and get the next character
			lex_save(lex, &save);
			c = lex_next_char(lex);
		// Do this until the character is not a digit
		} while ('0' <= c && c <= '9');

		// Revert to last character and set the type
		lex_revert(lex, &save);
		token->type = LEX_TYPE_DECIMAL;
	}
	return true;
}

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
		else if (c == '.' || c == 'e' || c == 'E')
		{
			// Check if it's a decimal and backtrack if not
			if (!create_decimal(lex, token, c == '.') && token->type != LEX_TYPE_INVALID)
				lex_revert(lex, &save);
			break;
		} else
		{
			// No longer an integer
			lex_revert(lex, &save);
			break;
		}
	}
}

// create_symbol(lexer_t*, lexeme_t*) -> void
// Creates a symbol.
void create_symbol(lexer_t* lex, lexeme_t* token)
{
	token->type = LEX_TYPE_SYMBOL;

	// Save the current state
	lexer_t save;
	lex_save(lex, &save);

	while (true)
	{
		// Get the next character
		char c = lex_next_char(lex);

		// Still a symbol
		if (c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'))
			lex_save(lex, &save);
		else if (c == '\'')
		{
			do
			{
				// Get all 's
				lex_save(lex, &save);
			} while (lex_next_char(lex) == '\'');

			// Revert to last character and exit
			lex_revert(lex, &save);
			break;
		} else
		{
			// No longer a symbol
			lex_revert(lex, &save);
			break;
		}
	}
}

// actual_symbol(lexeme_t*) -> void
// Checks if the token is actually a symbol or if it's instead a primative or keyword.
void actual_symbol(lexeme_t* token)
{
	// Check for keywords
	if (!strcmp(token->string, "for")
	 || !strcmp(token->string, "all")
	 || !strcmp(token->string, "some")
	 || !strcmp(token->string, "if")
	 || !strcmp(token->string, "then")
	 || !strcmp(token->string, "else")
	 || !strcmp(token->string, "such")
	 || !strcmp(token->string, "that")
	 || !strcmp(token->string, "in")
	 || !strcmp(token->string, "and")
	 || !strcmp(token->string, "or")
	 || !strcmp(token->string, "with"))
		token->type = LEX_TYPE_KEYWORD;

	// Check for primatives
	else if (!strcmp(token->string, "true")
		  || !strcmp(token->string, "false")
		  || !strcmp(token->string, "nil")
		  || !strcmp(token->string, "pass")
		  || !strcmp(token->string, "stop"))
		token->type = LEX_TYPE_PRIMATIVE;
}

// curly_lexer_func(lexer_t*) -> lexeme_t
// The lexer function used for lexing curly programs.
lexeme_t curly_lexer_func(lexer_t* lex)
{
	// Skip whitespace
	skip_whitespace_and_comments(lex);

	// Empty token
	lexeme_t token = init_lexeme(lex);

	// Check for eof
	if (lex_eof(lex))
	{
		token.type = LEX_TYPE_EOF;
		return token;
	}

	// Save the current state
	lexer_t save;
	lex_save(lex, &save);

	// Determine what to do based on the character received
	char c = lex_next_char(lex);
	if ('0' <= c && c <= '9')
		create_int(lex, &token);
	else if (c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
		create_symbol(lex, &token);
	else if (c == '-')
	{
		token.type = LEX_TYPE_INFIX_LEVEL_ADD;
		lexer_t s2;
		lex_save(lex, &s2);

		// Check if its a - or a negative number
		c = lex_next_char(lex);
		if ('0' <= c && c <= '9')
			create_int(lex, &token);
		else lex_revert(lex, &s2);
	} else if (c == '+')
		token.type = LEX_TYPE_INFIX_LEVEL_ADD;
	else if (c == '*' || c == '/' || c == '%')
		token.type = LEX_TYPE_INFIX_LEVEL_MUL;
	else if (c == '?')
		token.type = LEX_TYPE_POSTFIX;
	else if (c == '!')
	{
		token.type = LEX_TYPE_POSTFIX;
		lexer_t s2;
		lex_save(lex, &s2);

		// Check for inequality (!=)
		c = lex_next_char(lex);
		if (c == '=')
			token.type = LEX_TYPE_INFIX_LEVEL_COMPARE;
		else lex_revert(lex, &s2);
	} else if (c == '<' || c == '>')
	{
		token.type = LEX_TYPE_INFIX_LEVEL_COMPARE;
		lexer_t s2;
		lex_save(lex, &s2);

		// Check for equality (<=, >=)
		c = lex_next_char(lex);
		if (c != '=')
			lex_revert(lex, &s2);
	} else if (c == '=')
	{
		token.type = LEX_TYPE_SPECIAL_CHAR;
		lexer_t s2;
		lex_save(lex, &s2);

		// Check for equality (==)
		c = lex_next_char(lex);
		if (c == '=')
			token.type = LEX_TYPE_INFIX_LEVEL_COMPARE;
		else lex_revert(lex, &s2);
	} else if (c == ',' || c == ':' || c == '\\')
		token.type = LEX_TYPE_SPECIAL_CHAR;
	else if (c == '.')
	{
		lexer_t s2;
		lex_save(lex, &s2);

		// Check for range (..)
		c = lex_next_char(lex);
		if (c == '.')
			token.type = LEX_TYPE_SPECIAL_CHAR;
		else lex_revert(lex, &s2);
	} else if (c == '(' || c == '[' || c == '{'
		  || c == ')' || c == ']' || c == '}')
		token.type = LEX_TYPE_GROUPING;
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

	// Check if the token is actually a symbol
	if (token.type == LEX_TYPE_SYMBOL)
		actual_symbol(&token);

	return token;
}

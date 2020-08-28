//
// parse
// lexer.c: Implements the lexer for the language.
// 
// Created by jenra.
// Created on July 27 2020.
// 

#include <stdbool.h>
#include <string.h>

#include "lexer.h"
#include "../../../utils/list.h"

// init_lexer(lexer_t*, char*) -> void
// Initialises a lexer.
void init_lexer(lexer_t* lex, char* string)
{
	lex->string = strdup(string);
	lex->pos = 0;
	lex->lino = 1;
	lex->charpos = 0;
	lex->size = 16;
	lex->tokens = calloc(lex->size, sizeof(token_t));
	lex->count = 0;
}

// lex_skip_whitespace(lexer_t*) -> void
// Skips whitespace before a token.
void lex_skip_whitespace(lexer_t* lex)
{
	char c;
	bool comment = false;

	while (true)
	{
		// Get the next character
		c = lex->string[lex->pos];

		// Check for newline
		if (c == '\n')
		{
			lex->pos++;
			lex->charpos = 0;
			lex->lino++;
			comment = false;

		// Check for whitespace
		} else if (comment || c == ' ' || c == '\t' || c == '\r' || c == '#')
		{
			lex->pos++;
			lex->charpos++;
			comment = comment || c == '#';

		// Everything else should not be skipped
		} else break;
	}
}

// lex_next(lexer_t*) -> token_t*
// Consumes the next token in the string.
token_t* lex_next(lexer_t* lex)
{
	// Return the next token in the list if it was previously generated
	if (lex->token_pos < lex->count)
		return lex->tokens + lex->token_pos++;

	// Skip whitespace
	lex_skip_whitespace(lex);

	// Set up
	size_t i = lex->pos;
	token_t token;
	char c;
	bool iter = true;

	// Initialise the token
	token.type = LEX_TYPE_NONE;
	token.tag = LEX_TAG_NONE;
	token.value = NULL;
	token.pos = lex->pos;
	token.lino = lex->lino;
	token.charpos = lex->charpos;

	// Iterate over the string
	while (iter)
	{
		c = lex->string[i];

		switch (token.type)
		{
			case LEX_TYPE_NONE:
				// Break if a token type has not been assigned
				if (i != lex->pos)
					iter = false;

				// Determine the type of the token
				else if (c == '\0')
				{
					token.type = LEX_TYPE_EOF;
					iter = false;
				} else if ('0' <= c && c <= '9')
				{
					token.type = LEX_TYPE_INT;
					token.tag = LEX_TAG_OPERAND;
				} else if (c == '(' || c == '[' || c == '{')
				{
					token.type = LEX_TYPE_LGROUP;
					token.tag = LEX_TAG_OPERATOR;
				} else if (c == ')' || c == ']' || c == '}')
				{
					token.type = LEX_TYPE_RGROUP;
					token.tag = LEX_TAG_OPERATOR;
				} else if (c == ':')
				{
					token.type = LEX_TYPE_COLON;
					token.tag = LEX_TAG_OPERATOR;
				} else if (c == '\\')
				{
					token.type = LEX_TYPE_BACKSLASH;
					token.tag = LEX_TAG_OPERATOR;
				} else if (c == ',')
				{
					token.type = LEX_TYPE_COMMA;
					token.tag = LEX_TAG_OPERATOR;
				} else if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_' || c == '@')
				{
					token.type = LEX_TYPE_SYMBOL;
					token.tag = LEX_TAG_OPERAND;
				} else if (c == '=')
				{
					token.type = LEX_TYPE_ASSIGN;
					token.tag = LEX_TAG_OPERATOR;
				} else if (c == '<' || c == '>')
				{
					token.type = LEX_TYPE_COMPARE;
					token.tag = LEX_TAG_OPERATOR;
				} else if (c == '.')
				{
					token.type = LEX_TYPE_DOT;
					token.tag = LEX_TAG_OPERATOR;
				} else if (c == '*' || c == '/')
				{
					token.type = LEX_TYPE_MULDIV;
					token.tag = LEX_TAG_OPERATOR;
				} else if (c == '+' || c == '-')
				{
					token.type = LEX_TYPE_ADDSUB;
					token.tag = LEX_TAG_OPERATOR;
				}
				break;
			case LEX_TYPE_INT:
				// Turn ints into floats if necessary
				if (c == '.')
					token.type = LEX_TYPE_FLOAT;

				// Assert all characters in the token are digits
				else if (!('0' <= c && c <= '9'))
					iter = false;
				break;
			case LEX_TYPE_FLOAT:
				// Assert the float does not end with a dot
				if (lex->string[i - 1] == '.' && !('0' <= c && c <= '9'))
				{
					token.type = LEX_TYPE_NONE;
					token.tag = LEX_TAG_NONE;
				}

				// Assert all characters after the . in the token are digits
				else if (!('0' <= c && c <= '9'))
					iter = false;
				break;
			case LEX_TYPE_SYMBOL:
				// Assert only valid characters are in the symbol
				if (!(('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_'))
					iter = false;
				break;
			case LEX_TYPE_ASSIGN:
				// If there's another equal sign, then it's == (equal to)
				if (lex->string[i + 1] == '=')
					token.type = LEX_TYPE_COMPARE;
				else iter = false;
				break;
			case LEX_TYPE_COMPARE:
				// << and >> are operators
				if ((c == '<' || c == '>') && lex->string[i + 1] == c)
					token.type = LEX_TYPE_BITSHIFT;
				// <= and >= are comparison operators
				else if ((c != '<' && c != '>') || lex->string[i + 1] != '=')
					iter = false;
				break;
			case LEX_TYPE_DOT:
				// .. is the range operator
				if (lex->string[i + 1] == '.')
					token.type = LEX_TYPE_RANGE;
				else iter = false;
				break;

			// These token types are only one character long
			case LEX_TYPE_LGROUP:
			case LEX_TYPE_RGROUP:
			case LEX_TYPE_COLON:
			case LEX_TYPE_BACKSLASH:
			case LEX_TYPE_COMMA:
			case LEX_TYPE_RANGE:
			case LEX_TYPE_MULDIV:
			case LEX_TYPE_ADDSUB:
			default:
				iter = false;
				break;
		}

		// Increment appropriate variables if iterating
		if (iter)
		{
			i++;
			lex->charpos++;
		}
	}

	// Copy the value of the token into the token
	size_t length = i - lex->pos;
	token.value = malloc(length + 1);
	strncpy(token.value, lex->string + lex->pos, length);
	lex->pos = i + 1;

	// If the token is a symbol, check if the symbol is actually a keyword
	if (token.type == LEX_TYPE_SYMBOL && (
		!strcmp(token.value, "with")
	 || !strcmp(token.value, "for")
	 || !strcmp(token.value, "some")
	 || !strcmp(token.value, "all")
	 || !strcmp(token.value, "in")
	 || !strcmp(token.value, "if")
	 || !strcmp(token.value, "else")
	 || !strcmp(token.value, "where")
	 || !strcmp(token.value, "and")
	 || !strcmp(token.value, "or")
	 || !strcmp(token.value, "xor")
	 || !strcmp(token.value, "pass")
	 || !strcmp(token.value, "stop")))
	{
		token.type = LEX_TYPE_KEYWORD;
		token.tag = LEX_TAG_OPERATOR;
	}

	// Append the token to the list of tokens
	list_append_element(lex->tokens, lex->size, lex->count, token_t, token);
	lex->token_pos++;

	// Return a pointer to the token
	return lex->tokens + lex->count - 1;
}

// cleanup_lexer(lexer_t*) -> void
// Frees memory associated with the lexer.
void cleanup_lexer(lexer_t* lex)
{
	free(lex->string);

	for (size_t i = 0; i < lex->count; i++)
	{
		free(lex->tokens[i].value);
	}

	free(lex->tokens);
}

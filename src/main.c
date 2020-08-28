//
// Curly
// main.c: Runs the command line interface.
//
// jenra
// July 27 2020
//

#include <editline/readline.h>

#include "compiler/frontend/parse/lexer.h"

int main(int argc, char** argv)
{
	lexer_t lex;
	init_lexer(&lex, "2+3-4==5=6/symbol+keyword*x-y for all n in (x in iter where a == 2) debug n # this is a comment by the way \n other + stuff");

	token_t* token;

	while ((token = lex_next(&lex))->type != LEX_TYPE_EOF)
	{
		printf("%s (%i:%i/%i)\n", token->value, token->lino, token->charpos, token->type);
	}

	cleanup_lexer(&lex);
	return 0;
}

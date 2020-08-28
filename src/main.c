//
// Curly
// main.c: Runs the command line interface.
//
// jenra
// July 27 2020
//

#include <editline/readline.h>

#include "compiler/frontend/parse/lexer.h"
#include "compiler/frontend/parse/parser.h"

int main(int argc, char** argv)
{
	lexer_t lex;
	init_lexer(&lex, "x y: t*z=2");

	// token_t* token;
	// while ((token = lex_next(&lex))->type != LEX_TYPE_EOF)
	// {
	// 	printf("%s (%i:%i/%i)\n", token->value, token->lino, token->charpos, token->type);
	// }
	// lex.token_pos = 0;

	parse_result_t res = statement(&lex);

	if (res.succ)
		print_ast(res.ast);
	else
	{
		puts("an error occured");
		printf(res.error->message, res.error->value.value);
		printf(" (%i:%i)\n", res.error->value.lino, res.error->value.charpos);
	}

	cleanup_lexer(&lex);
	return 0;
}

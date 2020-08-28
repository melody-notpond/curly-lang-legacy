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
	init_lexer(&lex, "4*(4-5)/symbol*3 <= 4/5 == 3");

	parse_result_t res = expression(&lex);

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

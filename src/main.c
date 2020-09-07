//
// Curly
// main.c: Runs the command line interface.
//
// jenra
// July 27 2020
//

// #include <editline/readline.h>
#include <stdio.h>
#include <string.h>
#include "llvm-c/Analysis.h"

#include "compiler/frontend/correctness/check.h"
#include "compiler/frontend/parse/lexer.h"
#include "compiler/frontend/parse/parser.h"

int main(int argc, char** argv)
{
	if (argc == 2)
	{
		FILE* file = fopen(argv[1], "r");
		size_t size = 129;
		char* string = malloc(size);
		string[0] = '\0';

		while (!feof(file))
		{
			fgets(string + strlen(string), 64, file);
			if (strlen(string) > size - 2)
				string = realloc(string, (size <<= 1));
		}

		fclose(file);

		lexer_t lex;
		init_lexer(&lex, string);
		free(string);

		// token_t* token;
		// while ((token = lex_next(&lex))->type != LEX_TYPE_EOF)
		// {
		// 	printf("%s (%i:%i/%i)\n", token->value, token->lino, token->charpos, token->type);
		// }
		// lex.token_pos = 0;

		parse_result_t res = lang_parser(&lex);

		if (res.succ)
		{
			print_ast(res.ast);
			printf("Checking types\n");
			if (check_correctness(res.ast))
				printf("Check successful!\n");
			else printf("Check failed\n");
		} else
		{
			puts("an error occured");
			printf("Expected %s, got '%s'\n", res.error->expected, res.error->value.value);
			printf(" (%i:%i)\n", res.error->value.lino, res.error->value.charpos);
		}

		cleanup_lexer(&lex);
		clean_types();
	} else puts("usage: curly filename");
	return 0;
}

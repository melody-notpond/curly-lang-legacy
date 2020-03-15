//
// Curly
// main.c: Runs the command line interface.
//
// jenra
// March 3 2020
//

#include <editline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/backends/curlyvm/compile_bytecode.h"
#include "compiler/frontend/parser.h"
#include "vm/debug.h"
#include "vm/vm.h"

// repl(void) -> void
// Runs the repl.
void repl()
{
	puts("Curly repl (version 0)");
	parser_t parser = create_lang_parser();
	parse_result_t res;
	chunk_t chunk;
	globals_t globals;
	CurlyVM vm;

	globals.names = NULL;
	globals.size = 0;
	globals.count = 0;

	while (true)
	{
		// Get the string from user input
		char* input = readline("> ");
		if (input == NULL)
		{
			puts("");
			break;
		} else if (!strcmp(input, "quit"))
			break;

		// Parse the string
		res = parse_string(parser, input);
		print_parse_result(res);

		// Compile the result
		chunk = init_chunk();
		chunk.globals = globals;
		if (compile_tree(&chunk, &res, true))
		{
			// Disassemble
			disassemble(&chunk, "stdin");

			// Run the bytecode
			init_vm(&vm, &chunk);
			vm_run(&vm);
			clean_vm(&vm, false);
		}

		// Clean up
		clean_parse_result(&res);
		free(input);
	}

	clean_combinator(parser.comb);
}

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		repl();
	} else
	{
		// Initialise the parser
		parser_t parser = create_lang_parser();

		// Parse the file
		parse_result_t res = parse_file(parser, argv[1]);
		print_parse_result(res);

		// Compile the result
		chunk_t chunk = init_chunk();
		if (compile_tree(&chunk, &res, true))
		{
			// Disassemble
			disassemble(&chunk, argv[1]);

			// Run the bytecode
			CurlyVM vm;
			init_vm(&vm, &chunk);
			vm_run(&vm);
			clean_vm(&vm, true);
		}

		// Clean up
		clean_parse_result(&res);
		clean_combinator(parser.comb);
	}
	return 0;
}

//
// Curly
// main.c: Runs the command line interface.
//
// jenra
// March 3 2020
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/backends/curlyvm/compile_bytecode.h"
#include "compiler/frontend/parser.h"
#include "vm/debug.h"
#include "vm/vm.h"

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		puts("input must have at least one argument");
		return -1;
	}

	// Initialise the parser
	parser_t parser = create_lang_parser();

	// Parse the file
	parse_result_t res = parse_file(parser, argv[1]);
	print_parse_result(res);

	// Compile the result
	chunk_t chunk = init_chunk();
	compile_tree(&chunk, &res, true);

	// Disassemble
	disassemble(&chunk, argv[1]);

	// Run the bytecode
	CurlyVM vm;
	init_vm(&vm, &chunk);
	vm_run(&vm);

	// Clean up
	clean_vm(&vm);
	clean_parse_result(&res);
	clean_combinator(parser.comb);
	return 0;
}

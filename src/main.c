//
// Curly
// main.c: Runs the command line interface.
//
// jenra
// July 27 2020
//

#include <editline/readline.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/OrcBindings.h>
#include <stdio.h>
#include <string.h>

#include "compiler/backends/llvm/codegen.h"
#include "compiler/frontend/correctness/check.h"
#include "compiler/frontend/parse/lexer.h"
#include "compiler/frontend/parse/parser.h"

// count_groupings(char*, int) -> int
// Counts the number of unmatched grouping symbols and returns the result.
int count_groupings(char* string, int p)
{
	// Iterate over the string
	for (size_t i = 0; i < strlen(string); i++)
	{
		char c = string[i];

		// Increment if a left grouping symbol is found
		if (c == '(' || c == '[' || c == '{')
			p++;

		// Decrement if a right grouping symbol is found
		else if (c == ')' || c == ']' || c == '}')
		{
			p--;

			// Excessive right grouping symbols is an irrecoverable error
			if (p < 0)
				return p;
		}
	}
	return p;
}

int main(int argc, char** argv)
{
	// LLVMModuleRef mod = generate_code(NULL);
	// char* string = LLVMPrintModuleToString(mod);
	// printf("%s\n", string);
	// free(string);
	// LLVMDisposeModule(mod);
	// return 0;

	switch (argc)
	{
		case 1:
		{
			// // Set up
			// puts("Curly REPL");
			// ir_scope_t* scope = push_scope(NULL);
			// lexer_t lex;
			// parse_result_t res;
			// LLVMModuleRef mod = LLVMModuleCreateWithName("stdin");

			// // Init JIT
			// LLVMInitializeNativeTarget();
			// LLVMInitializeNativeAsmPrinter();
			// LLVMLinkInMCJIT();

			// // Create the execution engine
			// char* error = NULL;
			// LLVMExecutionEngineRef engine = NULL;
			// if (LLVMCreateJITCompilerForModule(&engine, mod, 0, &error) || error != NULL)
			// {
			// 	fprintf(stderr, "engine error: %s\n", error);
			// 	free(error);
			// 	LLVMDisposeExecutionEngine(engine);
			// 	return -1;
			// }

			// while (true)
			// {
			// 	// Get user input
			// 	char* input = readline(">>> ");
			// 	if (input == NULL || !strcmp(input, ":q") || !strcmp(input, ":quit"))
			// 	{
			// 		if (input == NULL) puts("");
			// 		free(input);
			// 		break;
			// 	}
			// 	add_history(input);

			// 	// Get next few lines if necessary
			// 	char c;
			// 	int p = count_groupings(input, 0);
			// 	while ((c = input[strlen(input) - 1]) == '\\' || c == ',' || p > 0)
			// 	{
			// 		// Read next line
			// 		char* next_line = readline("... ");
			// 		if (input == NULL || !strcmp(next_line, ""))
			// 		{
			// 			if (input == NULL) puts("");
			// 			free(input);
			// 			break;
			// 		}
			// 		add_history(next_line);
			// 		p = count_groupings(next_line, p);

			// 		// Concatenate
			// 		char* buffer = calloc(strlen(input) + strlen(next_line) + 6, 1);
			// 		strcat(buffer, input);
			// 		strcat(buffer, "\n    ");
			// 		strcat(buffer, next_line);
			// 		free(input);
			// 		free(next_line);
			// 		input = buffer;
			// 	}

			// 	// Init lexer
			// 	init_lexer(&lex, input);
			// 	free(input);

			// 	// Print out tokens
			// 	// token_t* token;
			// 	// while ((token = lex_next(&lex))->type != LEX_TYPE_EOF)
			// 	// {
			// 	// 	printf("%s (%i:%i/%i)\n", token->value, token->lino, token->charpos, token->type);
			// 	// }
			// 	// lex.token_pos = 0;

			// 	// Parse
			// 	res = lang_parser(&lex);

			// 	if (res.succ)
			// 	{
			// 		// Skip if no children
			// 		if (res.ast->children_count == 0)
			// 			continue;

			// 		// Type check
			// 		print_ast(res.ast);

			// 		// Build the LLVM IR if it's correct code
			// 		if (check_correctness(res.ast, scope))
			// 		{
			// 			LLVMValueRef main = generate_code(res.ast, mod);
			// 			char* modstr = LLVMPrintModuleToString(mod);
			// 			printf("%s\n", modstr);
			// 			free(modstr);

			// 			// Run the code
			// 			char* mainstr = LLVMPrintValueToString(main);
			// 			printf("%s\n", mainstr);
			// 			free(mainstr);
			// 			LLVMGenericValueRef ret = LLVMRunFunction(engine, main, 0, (LLVMGenericValueRef[]) {});

			// 			// Print the result
			// 			type_t* ret_type = res.ast->children[res.ast->children_count - 1]->type;
			// 			if (ret_type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ret_type->type_name, "Int"))
			// 				printf("  = %lli\n", LLVMGenericValueToInt(ret, true));
			// 			else if (ret_type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ret_type->type_name, "Float"))
			// 				printf("  = %.5f\n", LLVMGenericValueToFloat(LLVMDoubleType(), ret));
			// 			LLVMDisposeGenericValue(ret);
			// 		} else printf("Check failed\n");
			// 	} else
			// 	{
			// 		// Print out parsing error
			// 		puts("an error occured");
			// 		printf("Expected %s, got '%s'\n", res.error->expected, res.error->value.value);
			// 		printf(" (%i:%i)\n", res.error->value.lino, res.error->value.charpos);
			// 	}

			// 	// Clean up
			// 	cleanup_lexer(&lex);
			// 	clean_parse_result(res);
			// }

			// // Final clean up
			// clean_types();
			// pop_scope(scope);
			// LLVMDisposeModule(mod);
			// LLVMDisposeExecutionEngine(engine);
			// puts("Leaving Curly REPL");
			puts("REPL is not supported yet.");
			return -1;
		}
		case 2:
		{
			// Set up
			FILE* file = fopen(argv[1], "r");
			size_t size = 129;
			char* string = malloc(size);
			string[0] = '\0';

			// Read file
			while (!feof(file))
			{
				fgets(string + strlen(string), 64, file);
				if (strlen(string) > size - 2)
					string = realloc(string, (size <<= 1));
			}

			// Close file
			fclose(file);

			// Init lexer
			lexer_t lex;
			init_lexer(&lex, string);
			free(string);

			// Print out tokens
			// token_t* token;
			// while ((token = lex_next(&lex))->type != LEX_TYPE_EOF)
			// {
			// 	printf("%s (%i:%i/%i)\n", token->value, token->lino, token->charpos, token->type);
			// }
			// lex.token_pos = 0;

			// Parse
			parse_result_t res = lang_parser(&lex);

			if (res.succ)
			{
				// Type check
				print_ast(res.ast);
				if (check_correctness(res.ast, NULL))
				{
					// Build the LLVM IR
					LLVMModuleRef mod = LLVMModuleCreateWithName(argv[1]);
					LLVMValueRef main = generate_code(res.ast, mod);
					char* string = LLVMPrintModuleToString(mod);
					printf("%s", string);
					free(string);

					// Init JIT
					LLVMInitializeNativeTarget();
					LLVMInitializeNativeAsmPrinter();
					LLVMLinkInMCJIT();

					// Create the execution engine
					char* error = NULL;
					LLVMExecutionEngineRef engine = NULL;
					if (LLVMCreateJITCompilerForModule(&engine, mod, 0, &error) || error != NULL)
					{
						fprintf(stderr, "engine error: %s\n", error);
						free(error);
						LLVMDisposeExecutionEngine(engine);
						return -1;
					}

					// Run the code
					LLVMGenericValueRef ret = LLVMRunFunction(engine, main, 0, (LLVMGenericValueRef[]) {});

					// Print the result
					type_t* ret_type = res.ast->children[res.ast->children_count - 1]->type;
					if (ret_type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ret_type->type_name, "Int"))
						printf("  = %lli\n", LLVMGenericValueToInt(ret, true));
					else if (ret_type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ret_type->type_name, "Float"))
						printf("  = %.5f\n", LLVMGenericValueToFloat(LLVMDoubleType(), ret));

					LLVMDisposeGenericValue(ret);
					LLVMDisposeExecutionEngine(engine);
				} else printf("Check failed\n");
			} else
			{
				// Print out parsing error
				puts("an error occured");
				printf("Expected %s, got '%s'\n", res.error->expected, res.error->value.value);
				printf(" (%i:%i)\n", res.error->value.lino, res.error->value.charpos);
			}

			// Clean up
			cleanup_lexer(&lex);
			clean_parse_result(res);
			clean_types();
			return 0;
		}
		default:
			// Display usage message
			puts("usage: curly [filename]");
			return -1;
	}
}

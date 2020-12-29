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
#include "compiler/frontend/ir/generate_ir.h"
#include "compiler/frontend/parse/lexer.h"
#include "compiler/frontend/parse/parser.h"
#include "utils/list.h"

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

typedef union
{
	int64_t i64;
	double f64;
	bool i1;
	struct
	{
		int32_t reference_count;
		void* func;
		int8_t count;
		int8_t arity;
		int64_t thunk_bitmap;
		int64_t* args;
	} func_app;
} repl_value_t;

int main(int argc, char** argv)
{
	switch (argc)
	{
		case 1:
		{
			// Set up
			puts("Curly REPL");
			ir_scope_t* scope = push_scope(NULL);
			create_primatives(scope);
			lexer_t lex;
			parse_result_t res;
			curly_ir_t ir;
			init_ir(&ir);
			LLVMContextRef context = LLVMContextCreate();
			llvm_codegen_env_t* env = create_llvm_codegen_environment(LLVMModuleCreateWithNameInContext("repl-header", context));
			size_t globals_size = 0;
			size_t globals_count = 0;
			repl_value_t* global_vals = calloc(globals_size, sizeof(repl_value_t));
			repl_value_t last_repl_val = {0};

			// Init JIT
			LLVMInitializeNativeTarget();
			LLVMInitializeNativeAsmPrinter();
			LLVMLinkInMCJIT();

			while (true)
			{
				// Get user input
				char* input = readline(">>> ");
				if (input == NULL || !strcmp(input, ":q") || !strcmp(input, ":quit"))
				{
					if (input == NULL) puts("");
					free(input);
					break;
				}
				add_history(input);

				// Get next few lines if necessary
				char c;
				int p = count_groupings(input, 0);
				while ((c = input[strlen(input) - 1]) == '\\' || c == ',' || p > 0)
				{
					// Read next line
					char* next_line = readline("... ");
					if (input == NULL || !strcmp(next_line, ""))
					{
						if (input == NULL) puts("");
						free(input);
						break;
					}
					add_history(next_line);
					p = count_groupings(next_line, p);

					// Concatenate
					char* buffer = calloc(strlen(input) + strlen(next_line) + 2, 1);
					strcat(buffer, input);
					strcat(buffer, "\n");
					strcat(buffer, next_line);
					free(input);
					free(next_line);
					input = buffer;
				}

				// Init lexer
				init_lexer(&lex, input);
				free(input);

				// Print out tokens
				// token_t* token;
				// while ((token = lex_next(&lex))->type != LEX_TYPE_EOF)
				// {
				// 	printf("%s (%i:%i/%i)\n", token->value, token->lino, token->charpos, token->type);
				// }
				// lex.token_pos = 0;

				// Parse
				res = lang_parser(&lex);

				if (res.succ)
				{
					// Skip if no children
					if (res.ast->children_count == 0)
						continue;

					// Print
					print_ast(res.ast);

					// Generate IR code
					convert_ast_to_ir(res.ast, scope, &ir);
					print_ir(ir);

					// Type check
					// Build the LLVM IR if it's correct code
					if (check_correctness(ir, scope))
					{
						print_ir(ir);

						generate_code(ir, env);
						char* modstr = LLVMPrintModuleToString(env->header_mod);
						printf("%s\n", modstr);
						free(modstr);
						modstr = LLVMPrintModuleToString(env->body_mod);
						printf("%s\n", modstr);
						free(modstr);

						// Create the execution engine
						char* error = NULL;
						LLVMExecutionEngineRef engine = NULL;
						if (LLVMCreateExecutionEngineForModule(&engine, env->body_mod, &error) || error != NULL)
						{
							fprintf(stderr, "engine error: %s\n", error);
							free(error);
							LLVMDisposeExecutionEngine(engine);
							return -1;
						}

						// Set global mappings
						LLVMValueRef global = LLVMGetFirstGlobal(env->header_mod);
						size_t i = 0;
						while (global != NULL)
						{
							size_t length = 0;
							if (strcmp(LLVMGetValueName2(global, &length), "repl.last"))
							{
								if (i < globals_count)
									list_append_element(global_vals, globals_size, globals_count, repl_value_t, (repl_value_t) {0});
								LLVMAddGlobalMapping(engine, global, global_vals + i++);

							// The repl value global has its own special global value
							} else LLVMAddGlobalMapping(engine, global, &last_repl_val);

							global = LLVMGetNextGlobal(global);
						}

						// Run the code
						LLVMAddModule(engine, env->header_mod);
						LLVMDisposeGenericValue(LLVMRunFunction(engine, env->main_func, 0, (LLVMGenericValueRef[]) {}));

						// Print the result
						type_t* ret_type = ir.expr[ir.expr_count - 1]->type;
						printf("  = ");
						if (ret_type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ret_type->type_name, "Int"))
							printf("%li", last_repl_val.i64);
						else if (ret_type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ret_type->type_name, "Float"))
							printf("%.5f", last_repl_val.f64);
						else if (ret_type->type_type == IR_TYPES_PRIMITIVE && !strcmp(ret_type->type_name, "Bool"))
							printf("%s", last_repl_val.i1 ? "true" : "false");
						else if (ret_type->type_type == IR_TYPES_FUNC)
							printf("(%i) %p: %i/%i args, bitmap = %li, args => %p", last_repl_val.func_app.reference_count, last_repl_val.func_app.func, last_repl_val.func_app.count, last_repl_val.func_app.arity, last_repl_val.func_app.thunk_bitmap, last_repl_val.func_app.args);
						else printf("unknown value");
						puts("");

						// Clean up
						LLVMRemoveModule(engine, env->header_mod, &env->header_mod, &error);
						empty_llvm_codegen_environment(env);
						LLVMDisposeExecutionEngine(engine);
					} else printf("Check failed\n");

					clean_ir(&ir);
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
			}

			// Final clean up
			clean_functions(&ir);
			clean_types();
			pop_scope(scope);
			free(global_vals);
			clean_llvm_codegen_environment(env);
			LLVMContextDispose(context);
			puts("Leaving Curly REPL");
			return 0;
		}
		case 2:
		{
			// Set up
			FILE* file = fopen(argv[1], "r");
			size_t size = 128;
			char* string = malloc(size + 1);
			string[0] = '\0';

			// Read file
			while (!feof(file))
			{
				size_t max = size - strlen(string);
				fgets(string + strlen(string), max < 64 ? max : 64, file);
				if (strlen(string) + 1 >= size)
					string = realloc(string, (size <<= 1) + 1);
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
				print_ast(res.ast);

				// Generate IR code
				ir_scope_t* scope = push_scope(NULL);
				curly_ir_t ir;
				init_ir(&ir);
				convert_ast_to_ir(res.ast, scope, &ir);
				print_ir(ir);

				// Type check
				if (check_correctness(ir, NULL))
				{
					// Build the LLVM IR
					llvm_codegen_env_t* env = generate_code(ir, NULL);
					char* string = LLVMPrintModuleToString(env->body_mod);
					printf("%s", string);
					free(string);

					// Init JIT
					LLVMInitializeNativeTarget();
					LLVMInitializeNativeAsmPrinter();
					LLVMLinkInMCJIT();

					// Create the execution engine
					char* error = NULL;
					LLVMExecutionEngineRef engine = NULL;
					if (LLVMCreateJITCompilerForModule(&engine, env->body_mod, 0, &error) || error != NULL)
					{
						fprintf(stderr, "engine error: %s\n", error);
						free(error);
						LLVMDisposeExecutionEngine(engine);
						return -1;
					}

					// Run the code
					LLVMDisposeGenericValue(LLVMRunFunction(engine, env->main_func, 0, (LLVMGenericValueRef[]) {}));

					// Clean up
					LLVMDisposeExecutionEngine(engine);
					clean_llvm_codegen_environment(env);
				} else printf("Check failed\n");

				clean_functions(&ir);
				clean_ir(&ir);
				pop_scope(scope);
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

//
// Curly parser combinator
// combinator.c: Implements parser combinators.
//
// jenra
// February 25 2020
//

#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "combinator.h"

// init_combinator(void) -> comb_t*
// Initialises an empty parser.
comb_t* init_combinator()
{
	comb_t* comb = malloc(sizeof(comb_t));
	comb->func = NULL;
	comb->ignore_whitespace = false;
	comb->args = NULL;
	comb->next = NULL;
	return comb;
}

// init_ast_node(char*, int, int) -> ast_t
// Initialises a child ast node.
ast_t init_ast_node(char* value, int line, int char_pos)
{
	ast_t node;
	node.name = NULL;
	node.value = (value != NULL ? strdup(value) : NULL);
	node.line = line;
	node.char_pos = char_pos;
	node.children_count = 0;
	node.children = NULL;
	return node;
}

// init_ast_node(size_t, int, int) -> ast_t
// Initialises a parent ast node.
ast_t init_parent_node(size_t init_size, int line, int char_pos)
{
	ast_t node;
	node.name = NULL;
	node.value = NULL;
	node.line = line;
	node.char_pos = char_pos;
	node.children_count = 0;
	node.children = calloc(init_size, sizeof(ast_t));
	return node;
}

// init_empty_result(void) -> void
// Initialises an empty parse result that's ignored by parsers.
parse_result_t init_empty_result()
{
	parse_result_t result;
	result.succ = true;
	result.ignore = true;
	result.ast.name = NULL;
	result.ast.value = NULL;
	result.ast.line = 0;
	result.ast.char_pos = 0;
	result.ast.children_count = 0;
	result.ast.children = NULL;
	return result;
}

// init_succ_result(ast_t) -> parse_result_t
// Initialises a successful parse result.
parse_result_t init_succ_result(ast_t node)
{
	parse_result_t result;
	result.succ = true;
	result.ignore = false;
	result.ast = node;
	return result;
}

// init_error_result(char*, int, int)
// Initialises an erroneous parse result.
parse_result_t init_error_result(char* msg, int line, int char_pos)
{
	parse_result_t result;
	result.succ = false;
	result.ignore = false;
	result.error.msg = (msg != NULL ? strdup(msg) : NULL);
	result.error.line = line;
	result.error.char_pos = char_pos;
	return result;
}

//
// ================================================
//
// Start of parser combinator functions
//
// ================================================
//

// _match_str_func(lexer_t*, void*) -> void
// Comb function for c_str.
parse_result_t _match_str_func(lexer_t* lex, void* args)
{
	// Skip whitespace
	lex_skip_whitespace(lex);

	// Get the string
	char* to_match = (char*) args;

	// Save the state just in case
	lexer_t _lex = *lex;

	// Match the string one character at a time
	for (int i = 0; i < strlen(to_match); i++)
	{
		if (to_match[i] != lex_next(&_lex))
			return init_error_result(NULL, lex->line, lex->char_pos);
	}

	// Set up for returning
	ast_t node = init_ast_node(to_match, lex->line, lex->char_pos);
	*lex = _lex;
	return init_succ_result(node);
}

// c_str(char*) -> comb_t*
// Creates a parser valid for a given string.
comb_t* c_str(char* str)
{
	comb_t* comb = init_combinator();
	comb->func = _match_str_func;
	comb->args = malloc(strlen(str) + 1);
	strcpy((char*) comb->args, str);
	return comb;
}

// _match_regex_func(lexer_t*, void*) -> parse_result_t
// Comb function for c_regex.
parse_result_t _match_regex_func(lexer_t* lex, void* args)
{
	// Skip whitespace
	lex_skip_whitespace(lex);

	// Execute the regex
	regmatch_t match;
	int status = regexec((regex_t*) args, lex->string + lex->position, 1, &match, 0);

	// Check for error
	if (status != 0)
		return init_error_result(NULL, lex->line, lex->char_pos);

	// Create the ast node
	ast_t node = init_ast_node(NULL, lex->line, lex->char_pos);
	node.value = malloc(match.rm_eo + 1);
	strncpy(node.value, lex->string + lex->position, match.rm_eo);

	// Update lexer position
	lex->position += (int) match.rm_eo;

	return init_succ_result(node);
}

// c_regex(char*) -> comb_t*
// Creates a parser valid for a given regex.
comb_t* c_regex(char* pattern)
{
	// No null patterns allowed
	if (pattern == NULL)
	{
		puts("Invalid regex: (null)");
		exit(1);
	} else if (pattern[0] != '^')
	{
		// Add a starting anchor to the pattern so the regex only searches at the start
		char* temp = pattern;
		pattern = malloc(strlen(pattern) + 2);
		pattern[0] = '^';
		strcpy(pattern + 1, temp);
	}

	// Compile the regex
	regex_t* regex = malloc(sizeof(regex_t));
	int status = regcomp(regex, pattern, REG_EXTENDED);

	// Error if invalid regex
	if (status != 0)
	{
		size_t length = regerror(status, regex, NULL, 0);
		char* buffer = malloc(length);
		regerror(status, regex, buffer, length);
		printf("%s\n", buffer);
		free(regex);
		free(buffer);
		exit(1);
	}

	// Create the comb
	comb_t* comb = init_combinator();
	comb->func = _match_regex_func;
	comb->args = (void*) regex;
	return comb;
}

// get_combs_list(comb_t*, comb_t*, va_list) -> void*
// Generates a list of combinators from a va_list prefixed with its size.
void* get_combs_list(comb_t* c1, comb_t* c2, va_list ap)
{
	// Allocate space for the list
	size_t current_size = 10;
	void* result = malloc(sizeof(int) + current_size * sizeof(comb_t*));
	comb_t** args = (comb_t**) ((int*) result + 1);
	args[0] = c1;
	args[1] = c2;

	// Our iterators
	comb_t* next;
	unsigned int count = 2;

	// Get the next comb whilst it exists
	while ((next = va_arg(ap, comb_t*)) != NULL)
	{
		// Resize the list if necessary
		if (current_size <= count)
		{
			result = realloc(result, sizeof(int) + (current_size *= 1.9) * sizeof(comb_t*));
			args = (comb_t**) ((int*) result + 1);
		}

		// Append the comb
		args[count++] = next;
	}

	// Add the size to the beginning of the list
	int* count_ptr = (int*) result;
	count_ptr[0] = count;

	// Resize the list and return
	result = realloc(result, sizeof(int) + count * sizeof(comb_t*));
	return result;
}

// _match_or_func(lexer_t*, void*) -> void
// Comb function for c_or.
parse_result_t _match_or_func(lexer_t* lex, void* args)
{
	// Get the list
	unsigned int count = ((int*) args)[0];
	comb_t** combs = (comb_t**) (args + sizeof(int));

	// Iterate over the combs
	parse_result_t result;
	for (int i = 0; i < count; i++)
	{
		// Clean up if necessary
		if (i != 0)
			clean_parse_result(&result);

		// Try the next comb
		comb_t* comb = combs[i];
		result = comb->func(lex, comb->args);
		if (result.succ)
			return result;
	}

	// Error :(
	return result;
}

// c_or(comb_t*, comb_t*, ...) -> comb_t*
// Creates a parser valid if any given parser is valid.
// comb_t* list must end with NULL.
comb_t* c_or(comb_t* c1, comb_t* c2, ...)
{
	// Generate the comb list
	va_list ap;
	va_start(ap, c2);
	void* args = get_combs_list(c1, c2, ap);
	va_end(ap);

	// Create the comb
	comb_t* comb = init_combinator();
	comb->func = _match_or_func;
	comb->args = args;
	return comb;
}

// _match_seq_func(lexer_t*, void*) -> void
// Comb function for c_seq.
parse_result_t _match_seq_func(lexer_t* lex, void* args)
{
	// Get the list
	unsigned int count = ((int*) args)[0];
	comb_t** combs = (comb_t**) ((int*) args + 1);

	// Create the parent node
	ast_t ast = init_parent_node(count, lex->line, lex->char_pos);
	size_t current_size = count;
	lexer_t _lex = *lex;

	// Iterate over every comb
	for (int i = 0; i < count; i++)
	{
		// Parse the comb
		comb_t* comb = combs[i];
		parse_result_t result = comb->func(&_lex, comb->args);

		if (result.ignore)
		{
			// Ignore these
			clean_parse_result(&result);
			continue;
		} else if (!result.succ)
		{
			// Error :(
			clean_ast_node(&ast);
			return result;
		}

		// Unnamed parent nodes have their children stolen
		// This is because you don't want something like /ab*/ to have /b*/ with a separate parent
		if (result.ast.name == NULL && result.ast.children_count > 0)
		{
			// Resize the array to fit the unexpected children
			ast.children = realloc(ast.children, (current_size += result.ast.children_count) * sizeof(ast_t));

			// Append all children
			for (int j = 0; j < result.ast.children_count; j++)
			{
				ast.children[ast.children_count++] = result.ast.children[j];
			}

			// Free the children
			free(result.ast.children);
		} else
			// Append the result to the array
			ast.children[ast.children_count++] = result.ast;
	}

	// Save
	*lex = _lex;

	// If one child, just return the child without the parent
	if (ast.children_count == 1)
	{
		ast_t temp = ast.children[0];
		free(ast.children);
		ast = temp;
	}

	// Success! :D
	return init_succ_result(ast);
}

// c_seq(comb_t*, comb_t*, ...) -> comb_t*
// Creates a parser valid if all given parsers are valid in order.
// comb_t* list must end with NULL.
comb_t* c_seq(comb_t* c1, comb_t* c2, ...)
{
	// Generate the comb list
	va_list ap;
	va_start(ap, c2);
	void* args = get_combs_list(c1, c2, ap);
	va_end(ap);

	// Create the comb
	comb_t* comb = init_combinator();
	comb->func = _match_seq_func;
	comb->args = args;
	return comb;
}

// _match_zmore_func(lexer_t*, void*) -> void
// Comb function for c_zmore.
parse_result_t _match_zmore_func(lexer_t* lex, void* args)
{
	// Get the parser
	comb_t* comb = (comb_t*) args;
	parse_result_t res;

	// Parent node
	int current_size = 8;
	ast_t ast = init_parent_node(current_size, lex->line, lex->char_pos);

	// Iterate whilst the parser is still valid
	while ((res = comb->func(lex, comb->args)).succ)
	{
		if (res.ignore)
		{
			// Ignore these
			clean_parse_result(&res);
			continue;
		}

		// Resize the array whenever necessary
		if (current_size <= ast.children_count)
			ast.children = realloc(ast.children, (current_size <<= 1) * sizeof(ast_t));

		// Unnamed parent nodes have their children stolen
		// This is because you don't want something like /(ab)*/ to have /ab/ with a separate parent every time
		if (res.ast.name == NULL && res.ast.value == NULL && res.ast.children_count > 0)
		{
			// Resize the array to fit the unexpected children
			if (current_size <= ast.children_count + res.ast.children_count)
				ast.children = realloc(ast.children, (current_size += res.ast.children_count) * sizeof(ast_t));

			// Append all children
			for (int j = 0; j < res.ast.children_count; j++)
			{
				ast.children[ast.children_count++] = res.ast.children[j];
			}

			// Free the children
			free(res.ast.children);
		} else
			// Append the result to the array
			ast.children[ast.children_count++] = res.ast;
	}

	if (ast.children_count == 0)
	{
		// Zero nodes isn't a bad thing
		clean_ast_node(&ast);
		res = init_empty_result();
	} else if (ast.children_count == 1)
	{
		// There was just one, so return the child node
		res = init_succ_result(ast.children[0]);
		free(ast.children);
	} else
		// Return the entire thing
		res = init_succ_result(ast);

	return res;
}

// c_zmore(comb_t*) -> comb_t*
// Creats a parser valid if a given parser appears at least zero times.
comb_t* c_zmore(comb_t* c)
{
	comb_t* comb = init_combinator();
	comb->func = _match_zmore_func;
	comb->args = (void*) c;
	return comb;
}

// _match_omore_func(lexer_t*, void*) -> void
// Comb function for c_omore.
parse_result_t _match_omore_func(lexer_t* lex, void* args)
{
	// Get the parser
	comb_t* comb = (comb_t*) args;
	parse_result_t res;

	// Parent node
	int current_size = 8;
	ast_t ast = init_parent_node(current_size, lex->line, lex->char_pos);

	// Iterate whilst the parser is still valid
	while ((res = comb->func(lex, comb->args)).succ)
	{
		if (res.ignore)
		{
			// Ignore these
			clean_parse_result(&res);
			continue;
		}

		// Resize the array whenever necessary
		if (current_size <= ast.children_count)
			ast.children = realloc(ast.children, (current_size <<= 1) * sizeof(ast_t));

		// Unnamed parent nodes have their children stolen
		// This is because you don't want something like /(ab)+/ to have /ab/ with a separate parent every time
		if (res.ast.name == NULL && res.ast.value == NULL && res.ast.children_count > 0)
		{
			// Resize the array to fit the unexpected children
			if (current_size <= ast.children_count + res.ast.children_count)
				ast.children = realloc(ast.children, (current_size += res.ast.children_count) * sizeof(ast_t));

			// Append all children
			for (int j = 0; j < res.ast.children_count; j++)
			{
				ast.children[ast.children_count++] = res.ast.children[j];
			}

			// Free the children
			free(res.ast.children);
		} else
			// Append the result to the array
			ast.children[ast.children_count++] = res.ast;
	}

	// Zero nodes is a problem
	if (ast.children_count == 0)
		clean_ast_node(&ast);
	else if (ast.children_count == 1)
	{
		// There was just one, so return the child node
		res = init_succ_result(ast.children[0]);
		free(ast.children);
	} else
		// Return the entire thing
		res = init_succ_result(ast);

	return res;
}

// c_zmore(comb_t*) -> comb_t*
// Creats a parser valid if a given parser appears at least once.
comb_t* c_omore(comb_t* c)
{
	comb_t* comb = init_combinator();
	comb->func = _match_omore_func;
	comb->args = (void*) c;
	return comb;
}

// _match_optional_func(lexer_t*, void*) -> void
// Comb function for c_optional.
parse_result_t _match_optional_func(lexer_t* lex, void* args)
{
	// Parse
	comb_t* comb = (comb_t*) args;
	parse_result_t result = comb->func(lex, comb->args);

	// Success!
	if (result.succ)
		return result;
	
	// Failure isn't bad, so return an empty result
	clean_parse_result(&result);
	return init_empty_result();
}

// c_optional(comb_t*) -> comb_t*
// Creates a parser valid regardless of if the given parser is valid.
comb_t* c_optional(comb_t* c)
{
	comb_t* comb = init_combinator();
	comb->func = _match_optional_func;
	comb->args = (void*) c;
	return comb;
}

// _match_not_func(lexer_t*, void*) -> void
// Comb function for c_not.
parse_result_t _match_not_func(lexer_t* lex, void* args)
{
	// Get the parser
	comb_t* comb = (comb_t*) args;
	lexer_t _lex = *lex;

	// Parse
	parse_result_t result = comb->func(&_lex, comb->args);
	clean_parse_result(&result);

	// Success is failure
	if (result.succ)
		return init_error_result(NULL, lex->line, lex->char_pos);

	// Failure is success
	*lex = _lex;
	return init_empty_result();
}

// c_not(comb_t*) -> comb_t*
// Creates a parser valid if the given parser is invalid.
comb_t* c_not(comb_t* c)
{
	comb_t* comb = init_combinator();
	comb->func = _match_not_func;
	comb->args = (void*) c;
	return comb;
}

// _match_next_func(lexer_t*, void*) -> void
// Comb function for c_next.
parse_result_t _match_next_func(lexer_t* lex, void* args)
{
	// Skip whitespace
	lex_skip_whitespace(lex);

	// Just get the next character
	int line = lex->line;
	int char_pos = lex->char_pos;
	char string[] = {lex_next(lex), '\0'};
	return init_succ_result(init_ast_node(string, line, char_pos));
}

// c_next(void) -> comb_t*
// Creates a parser that's always valid.
comb_t* c_next()
{
	comb_t* comb = init_combinator();
	comb->func = _match_next_func;
	return comb;
}

// _match_ignore_func(lexer_t*, void*) -> void
// Comb function for c_ignore.
parse_result_t _match_ignore_func(lexer_t* lex, void* args)
{
	// Parse
	comb_t* comb = (comb_t*) args;
	parse_result_t result = comb->func(lex, comb->args);

	// Errors are still bad
	if (!result.succ)
		return result;
	
	// Ignore the result generated
	clean_parse_result(&result);
	return init_empty_result();
}

// c_ignore(comb_t*) -> comb_t*
// Creates a parser that ignores the output of a given parser.
// Validity is still dependent on the given parser.
comb_t* c_ignore(comb_t* c)
{
	comb_t* comb = init_combinator();
	comb->func = _match_ignore_func;
	comb->args = (void*) c;
	return comb;
}

// _match_eof_func(lexer_t*, void*) -> void
// Comb function for c_eof.
parse_result_t _match_eof_func(lexer_t* lex, void* args)
{
	// Skip whitespace
	lex_skip_whitespace(lex);

	lexer_t _lex = *lex;

	// Test for end of file
	if (lex_next(&_lex) == '\0')
		return init_succ_result(init_ast_node("", lex->line, lex->char_pos));
	
	// Error because the next character wasn't the end
	return init_error_result(NULL, lex->line, lex->char_pos);
}

// c_eof(void) -> comb_t*
// Creates a parser valid if end of file.
comb_t* c_eof()
{
	comb_t* comb = init_combinator();
	comb->func = _match_eof_func;
	return comb;
}

// _match_set_name_func(lexer_t*, void*) -> void
// Comb function for c_name.
parse_result_t _match_set_name_func(lexer_t* lex, void* args)
{
	// Parse
	comb_t* comb = ((comb_t**) args)[0];
	parse_result_t result = comb->func(lex, comb->args);

	if (result.succ)
	{
		// Make a new parent if the node already has a name
		if (result.ast.name != NULL)
		{
			ast_t new = init_parent_node(1, result.ast.line, result.ast.char_pos);
			new.children[0] = result.ast;
			new.children_count = 1;
			result.ast = new;
		}

		// Give the node a name
		result.ast.name = strdup((char*) (args + sizeof(comb_t*)));
	}
	return result;
}

// c_name(char*, comb_t*) -> comb_t*
// Creates a parser that gives a name to a given parser.
comb_t* c_name(char* name, comb_t* c)
{
	// Generate the arguments (comb + name)
	void** args = malloc(sizeof(comb_t*) + strlen(name) + 1);;
	args[0] = c;
	strcpy((char*) (args + 1), name);

	// Create the comb
	comb_t* comb = init_combinator();
	comb->func = _match_set_name_func;
	comb->args = args;
	return comb;
}

//
// ================================================
//
// End of parser combinator functions
//
// ================================================
//

// c_set(comb_t*, comb_t*) -> void
// Sets one parser to another.
//
// To use this, a must be an empty parser. This function is used as follows:
// comb_t* recursive_parser = init_combinator();
// comb_t* value = some_parser_stuff_in_terms_of(recursive_parser);
// c_set(recursive_parser, value);
//
void c_set(comb_t* a, comb_t* b)
{
	a->func = b->func;
	a->args = b->args;
	free(b);
}

// parse(comb_t*, char*) -> parse_result_t
// Parses a string and returns the result.
parse_result_t parse(comb_t* parser, char* string)
{
	lexer_t lex = lex_str(string, parser->ignore_whitespace);
	parse_result_t result = parser->func(&lex, parser->args);
	clean_lex(&lex);
	return result;
}

// ast_print_helper(ast_t, int) -> void
// Prints out one node of an ast and its children.
// The end result looks something like this:
// root (1:0)|>
//     "node1" (1:0)
//     "node2" (1:5)
//     node3 (1:8)|>
//         "node4" (1:8)
//     (eof) (1:12)
void ast_print_helper(ast_t node, int level)
{
	// Print out the appropriate amount indentation
	for (int i = 0; i < level; i++)
		printf("    ");

	// Print out the name if available
	if (node.name != NULL)
		printf("%s ", node.name);
	
	// Print out the value if available or necessary
	if (node.value != NULL && node.value[0] != '\0')
		printf("\"%s\" ", node.value);
	else if (node.value != NULL)
		printf("(eof) ");
	else if (node.name == NULL)
		printf("(null) ");
	
	// Print out the location in the file
	printf("(%i:%i)", node.line, node.char_pos);

	// Print out all children
	if (node.children_count != 0)
	{
		printf("|>");
		for (int i = 0; i < node.children_count; i++)
		{
			puts("");
			ast_print_helper(node.children[i], level + 1);
		}
	}
}

// print_parse_result(parse_result_t) -> void
// Prints out a parse result.
void print_parse_result(parse_result_t result)
{
	if (result.succ)
	{
		// Print out the ast tree
		ast_print_helper(result.ast, 0);
		puts("");
	} else
	{
		// Print out the error message
		if (result.error.msg != NULL)
			printf("Syntax error: %s", result.error.msg);
		else
			printf("Unknown syntax error");
		
		// Print the location in the file
		printf(" (%i:%i)\n", result.error.line, result.error.char_pos);
	}
}

// clean_parse_result(parse_result_t*) -> void
// Cleans up a parse result.
void clean_parse_result(parse_result_t* result)
{
	if (result == NULL)
		return;
	if (result->succ)
		// Clean up tree
		clean_ast_node(&(result->ast));
	else
	{
		// Clean up error
		free(result->error.msg);
		result->error.msg = NULL;
	}
}

// clean_ast_node(ast_t*) -> void
// Cleans up an ast node.
void clean_ast_node(ast_t* node)
{
	if (node == NULL)
		return;
	
	// Free all children
	if (node->children_count > 0)
	{
		for (int i = 0; i < node->children_count; i++)
		{
			clean_ast_node(node->children + i);
		}
		free(node->children);
		node->children = NULL;
		node->children_count = 0;
	}

	// Free the name
	if (node->name != NULL)
		free(node->name);

	// Free the value
	if (node->value != NULL)
		free(node->value);
}

// build_comb_list(comb_t*) -> comb_t*
// Builds a linked list of comb_t* for deletion.
comb_t* build_comb_list(comb_t* comb)
{
	comb_t* last = comb;
	
	while (true)
	{
		// Multiple combs to be appended
		if (last->func == _match_or_func || last->func == _match_seq_func)
		{
			// Get the number of combs and the list of combs
			int count = ((int*) last->args)[0];
			comb_t** combs = (comb_t**) (last->args + sizeof(int));

			for (int i = 0; i < count; i++)
			{
				comb_t* next = combs[i];

				// This condition tests if the comb was already added to the list
				if (next != last && next->next == NULL)
				{
					// Append next to the linked list
					last->next = next;

					// Build the link list starting with next and get the very last element
					last = build_comb_list(next);
				}
			}
			break;

		// One comb to be appended
		} else if (last->func == _match_zmore_func
				|| last->func == _match_omore_func
				|| last->func == _match_optional_func
				|| last->func == _match_not_func
				|| last->func == _match_ignore_func
				|| last->func == _match_set_name_func)
		{
			// Get the comb
			comb_t* next;
			if (last->func == _match_set_name_func)
				// c_name has a slightly different memory layout than the other combs
				next = ((comb_t**) last->args)[0];
			else
				next = (comb_t*) last->args;

			// This condition tests if the comb was already added to the list
			if (next != last && next->next == NULL)
			{
				// Append next to the linked list
				last->next = next;

				// Build the link list starting with next and get the very last element
				last = next;
			} else break;

		// No combs to be appended
		} else break;
	}

	// The last one is returned since this function builds the linked list recursively
	return last;
}

// clean_combinator(comb_t*) -> void
// Deletes a comb parser.
void clean_combinator(comb_t* comb)
{
	// Generate the linked list of combinators for deletion
	build_comb_list(comb);

	comb_t* current = comb;
	while (current != NULL)
	{
		// Free args if appropriate
		if (current->func == _match_str_func
		 || current->func == _match_or_func
		 || current->func == _match_seq_func
		 || current->func == _match_set_name_func)
			free(current->args);
		else if (current->func == _match_regex_func)
		{
			regfree((regex_t*) current->args);
			free(current->args);
		}

		// Get next comb and free current one
		comb_t* last = current;
		current = current->next;
		free(last);
	}
}

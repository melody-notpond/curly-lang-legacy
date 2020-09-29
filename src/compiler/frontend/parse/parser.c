// 
// parse
// parser.c: Implements the parser for curly.
// 
// Created by jenra.
// Created on August 28 2020.
// 

#include <string.h>

#include "../../../utils/list.h"
#include "parser.h"

// succ_result(ast_t*) -> parse_result_t
// Returns a parse result containing the ast node.
parse_result_t succ_result(ast_t* ast)
{
	parse_result_t res;
	res.succ = true;
	res.ast = ast;
	return res;
}

// err_result(bool, token_t, char*) -> parse_result_t
// Returns a parse result containing an error.
parse_result_t err_result(bool fatal, token_t token, char* expected)
{
	error_t* err = malloc(sizeof(error_t));
	err->fatal = fatal;
	err->value = token;
	err->value.value = token.value != NULL ? strdup(token.value) : NULL;
	err->expected = strdup(expected);

	parse_result_t res;
	res.succ = false;
	res.error = err;
	return res;
}

// consume_string(lexer_t*, char*, bool) -> parse_result_t
// Consumes a string from the lexer.
parse_result_t consume_string(lexer_t* lex, char* string, bool fatal)
{
	// Next token
	token_t* token = lex_next(lex);

	// Check if the token matches the string
	if (!strcmp(token->value, string))
		return succ_result(init_ast(*token));

	// Check for a lexer error
	else if (token->type == LEX_TYPE_NONE)
		return err_result(true, *token, string);

	// Return an error
	else return err_result(fatal, *token, string);
}

// consume_type(lexer_t*, lex_type_t, bool) -> parse_result_t
// Consumes a type from the lexer.
parse_result_t consume_type(lexer_t* lex, lex_type_t type, bool fatal)
{
	// Next token
	token_t* token = lex_next(lex);

	// Check if the token matches the type
	if (token->type == type)
		return succ_result(init_ast(*token));

	// Check for a lexer error
	else if (token->type == LEX_TYPE_NONE)
		return err_result(true, *token, lex_type_string(type));

	// Return an error
	else return err_result(fatal, *token, lex_type_string(type));
}

// consume_tag(lexer_t*, lex_tag_t, bool) -> parse_result_t
// Consumes a tag from the lexer.
parse_result_t consume_tag(lexer_t* lex, lex_tag_t tag, bool fatal)
{
	// Next token
	token_t* token = lex_next(lex);

	// Check if the token matches the tag
	if (token->tag == tag)
		return succ_result(init_ast(*token));

	// Check for a lexer error
	else if (token->type == LEX_TYPE_NONE)
		return err_result(true, *token, tag == LEX_TAG_OPERAND ? "operand" : "tag");

	// Return an error
	else return err_result(fatal, *token, tag == LEX_TAG_OPERAND ? "operand" : "tag");
}

#define push_lexer(lex) size_t prev_token_pos = lex->token_pos
#define repush_lexer(lex) prev_token_pos = lex->token_pos

// consume(parse_result_t, bool, string|type|tag, lexer_t*, ?, parse_result_t, bool) -> void
// Consumes a token from the lexer.
#define consume(res, required, type, lex, arg, built, fatal_)	\
	parse_result_t res = consume_##type(lex, arg, fatal_);		\
	if (!res.succ)												\
	{															\
		(lex)->token_pos = prev_token_pos;						\
		if (res.error->fatal || (required))						\
		{														\
			clean_parse_result(built);							\
			return res;											\
		}														\
	}

// call(parse_result_t, bool, func, lexer_t*, parse_result_t) -> void
// Calls a function and crashes if a fatal error occurs.
#define call(res, required, func, lex, built, fatal_)	\
	parse_result_t res = func(lex);						\
	if (!res.succ)										\
	{													\
		(lex)->token_pos = prev_token_pos;				\
		if (fatal_)										\
			res.error->fatal = fatal_;					\
		if (res.error->fatal || (required))				\
		{												\
			clean_parse_result(built);					\
			return res;									\
		}												\
	}

// expression: 'pass' | 'stop' | with_expr | if_expr | for_loop | xor
parse_result_t expression(lexer_t* lex);

// statement: assignment | expression
parse_result_t statement(lexer_t* lex);

// list_expr: '[' (for_loop | where_expr | (expression (',' expression?)*)?) ']'
parse_result_t list_expr(lexer_t* lex)
{
	// Push lexer
	push_lexer(lex);

	// Consume left bracket
	consume(lbrack, true, string, lex, "[", (parse_result_t) {false}, false);
	lbrack.ast->children_size = 1;
	lbrack.ast->children = calloc(1, sizeof(ast_t*));

	// Consume a newline
	repush_lexer(lex);
	consume(newline, false, type, lex, LEX_TYPE_NEWLINE, lbrack, false);
	clean_parse_result(newline);
	repush_lexer(lex);

	// Try to collect items for the list
	call(expr, false, expression, lex, lbrack, false);
	if (expr.succ)
	{
		// Collect items
		list_append_element(lbrack.ast->children, lbrack.ast->children_size, lbrack.ast->children_count, ast_t*, expr.ast);
		while (true)
		{
			// Push lexer
			push_lexer(lex);

			// Consume comma
			consume(comma, false, type, lex, LEX_TYPE_COMMA, lbrack, false);
			clean_parse_result(comma);
			if (!comma.succ) break;
			repush_lexer(lex);

			// Consume a newline
			repush_lexer(lex);
			consume(newline, false, type, lex, LEX_TYPE_NEWLINE, lbrack, false);
			clean_parse_result(newline);

			// Consume item
			call(expr, false, expression, lex, lbrack, false);
			if (expr.succ)
				list_append_element(lbrack.ast->children, lbrack.ast->children_size, lbrack.ast->children_count, ast_t*, expr.ast);
			else clean_parse_result(expr);
		}
	}

	// Consume a newline
	repush_lexer(lex);
	consume(newline2, false, type, lex, LEX_TYPE_NEWLINE, lbrack, false);
	clean_parse_result(newline2);

	// Consume right bracket
	consume(rbrack, true, string, lex, "]", lbrack, true);
	clean_parse_result(rbrack);
	return lbrack;
}

// dict_item: (symbol '=')? expression
parse_result_t dict_item(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume a symbol
	consume(symbol, false, type, lex, LEX_TYPE_SYMBOL, (parse_result_t) {false}, false);

	// Consume an equal sign and create the tree
	consume(assign, false, type, lex, LEX_TYPE_ASSIGN, symbol, false);

	if (symbol.succ && assign.succ)
	{
		assign.ast->children_size = 2;
		assign.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, symbol.ast);
	} else if (assign.succ)
	{
		clean_parse_result(assign);
		return symbol;
	} else
	{
		clean_parse_result(symbol);
		clean_parse_result(assign);
		assign.ast = NULL;
	}

	// Consume an expression
	call(expr, true, expression, lex, assign, true);
	if (!assign.succ)
		return expr;
	list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, expr.ast);
	return assign;
}

// dict_expr: '{' (dict_item (',' dict_item?)*)? '}'
parse_result_t dict_expr(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume curly brace
	consume(lcurly, true, string, lex, "{", (parse_result_t) {false}, false);

	// Consume a newline
	repush_lexer(lex);
	consume(newline, false, type, lex, LEX_TYPE_NEWLINE, lcurly, false);
	clean_parse_result(newline);

	// Try to collect items for the dictionary
	call(item, false, dict_item, lex, lcurly, false);
	if (item.succ)
	{
		// Collect items
		list_append_element(lcurly.ast->children, lcurly.ast->children_size, lcurly.ast->children_count, ast_t*, item.ast);
		while (true)
		{
			// Push lexer
			push_lexer(lex);

			// Consume comma
			consume(comma, false, type, lex, LEX_TYPE_COMMA, lcurly, false);
			clean_parse_result(comma);
			if (!comma.succ) break;
			repush_lexer(lex);

			// Consume a newline
			repush_lexer(lex);
			consume(newline, false, type, lex, LEX_TYPE_NEWLINE, lcurly, false);
			clean_parse_result(newline);

			// Consume item
			call(item, false, dict_item, lex, lcurly, false);
			if (item.succ)
				list_append_element(lcurly.ast->children, lcurly.ast->children_size, lcurly.ast->children_count, ast_t*, item.ast);
			else clean_parse_result(item);
		}
	} else clean_parse_result(item);

	// Consume a newline
	repush_lexer(lex);
	consume(newline2, false, type, lex, LEX_TYPE_NEWLINE, lcurly, false);
	clean_parse_result(newline2);

	// Consume right curly brace
	consume(rcurly, true, string, lex, "}", lcurly, true);
	clean_parse_result(rcurly);
	return lcurly;
}

// value: operand | '(' expression ')' | list_expr | dict_expr
parse_result_t value(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume an operand
	consume(res, false, tag, lex, LEX_TAG_OPERAND, (parse_result_t) {false}, false);

	// Return the operand if successful
	if (res.succ)
		return res;
	else clean_parse_result(res);

	// Try to consume a list
	call(list, false, list_expr, lex, (parse_result_t) {false}, false);
	if (list.succ)
		return list;
	else clean_parse_result(list);

	// Try to consume a dictionary
	call(dict, false, dict_expr, lex, (parse_result_t) {false}, false);
	if (dict.succ)
		return dict;
	else clean_parse_result(dict);

	// Consume a parenthesised expression
	consume(lparen, true, string, lex, "(", (parse_result_t) {false}, false);
	clean_parse_result(lparen);

	// Consume a newline
	repush_lexer(lex);
	consume(newline, false, type, lex, LEX_TYPE_NEWLINE, (parse_result_t) {false}, false);
	clean_parse_result(newline);

	// Consume expression
	call(expr, true, expression, lex, (parse_result_t) {false}, true);

	// Consume a newline
	repush_lexer(lex);
	consume(newline2, false, type, lex, LEX_TYPE_NEWLINE, expr, false);
	clean_parse_result(newline2);

	consume(rparen, true, string, lex, ")", expr, true);
	clean_parse_result(rparen);
	return expr;
}

#define infix_parser(name, subparser, type, operator, left_assoc)																\
parse_result_t name(lexer_t* lex)																								\
{																																\
	/* Push the lexer */																										\
	push_lexer(lex);																											\
																																\
	/* Get left operand */																										\
	call(top, true, subparser, lex, (parse_result_t) {false}, false);															\
	parse_result_t right_acc = top;																								\
	parse_result_t right_operand = top;																							\
																																\
	while (true)																												\
	{																															\
		/* Push the lexer */																									\
		push_lexer(lex);																										\
																																\
		/* Get operator */																										\
		consume(op, false, type, lex, operator, top, false);																	\
		if (!op.succ) { clean_parse_result(op); break; }																		\
																																\
		if (left_assoc)																											\
		{																														\
			/* Add left operand to the operator ast node */																		\
			op.ast->children_size = 2;																							\
			op.ast->children = calloc(2, sizeof(ast_t*));																		\
			list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t*, top.ast);				\
			top = op;																											\
		} else																													\
		{																														\
			/* Add operator to right operand */																					\
			op.ast->children_size = 2;																							\
			op.ast->children = calloc(2, sizeof(ast_t*));																		\
			list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t*, right_operand.ast);	\
			if (right_operand.ast == top.ast)																					\
			{																													\
				top = op;																										\
				right_acc = top;																								\
				right_operand.ast = top.ast->children[1];																		\
			}																													\
			right_acc.ast->children[1] = op.ast;																				\
			right_acc = op;																										\
		}																														\
																																\
		/* Get right operand */																									\
		call(right, true, subparser, lex, top, true);																			\
		list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t*, right.ast);				\
		right_operand = right;																									\
	}																															\
																																\
	/* Return the parsed expression */																							\
	return top;																													\
}

// attribute: prefix ('.' prefix)*
infix_parser(attribute, value, type, LEX_TYPE_DOT, true)

// application: attribute+
parse_result_t application(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume the function
	call(func, true, attribute, lex, (parse_result_t) {false}, false);

	// Consume any arguments if necessary
	while (true)
	{
		// Push lexer
		push_lexer(lex);

		// Get argument
		call(arg, false, attribute, lex, func, false);
		if (!arg.succ)
		{
			clean_parse_result(arg);
			break;
		}

		// Construct tree
		ast_t* app = init_ast((token_t) {LEX_TYPE_APPLICATION, LEX_TAG_OPERATOR, strdup("app"), func.ast->value.pos, func.ast->value.lino, func.ast->value.charpos});
		app->children_size = 2;
		app->children = calloc(2, sizeof(ast_t*));
		list_append_element(app->children, app->children_size, app->children_count, ast_t*, func.ast);
		list_append_element(app->children, app->children_size, app->children_count, ast_t*, arg.ast);
		func.ast = app;
	}

	return func;
}

// prefix: ('*' | '-')? application
parse_result_t prefix(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Try to consume curry operator
	consume(curry, false, string, lex, "*", (parse_result_t) {false}, false);
	if (curry.succ)
	{
		// Append a value to the curry operator
		call(app, true, application, lex, curry, true);
		curry.ast->children_size = 1;
		curry.ast->children = calloc(1, sizeof(ast_t*));
		list_append_element(curry.ast->children, curry.ast->children_size, curry.ast->children_count, ast_t*, app.ast);
		curry.ast->value.tag = LEX_TAG_OPERATOR;
		return curry;
	}

	// Try to consume negative operator
	clean_parse_result(curry);
	consume(negative, false, string, lex, "-", (parse_result_t) {false}, false);
	if (negative.succ)
	{
		// Append a value to the curry operator
		call(app, true, application, lex, negative, true);
		negative.ast->children_size = 1;
		negative.ast->children = calloc(1, sizeof(ast_t*));
		list_append_element(negative.ast->children, negative.ast->children_size, negative.ast->children_count, ast_t*, app.ast);
		negative.ast->value.tag = LEX_TAG_OPERATOR;
		return negative;
	}

	// Return the regular application if no prefix was found
	clean_parse_result(negative);
	return application(lex);
}

// muldiv: prefix (('*'|'/') prefix)*
infix_parser(muldiv, prefix, type, LEX_TYPE_MULDIV, true)

// addsub: muldiv (('+'|'-') muldiv)*
infix_parser(addsub, muldiv, type, LEX_TYPE_ADDSUB, true)

// bitshift: addsub (('<<'|'>>') addsub)*
infix_parser(bitshift, addsub, type, LEX_TYPE_BITSHIFT, true)

// typing: bitshift (':' bitshift)*
// Note that the code correctness checker will assert only one colon pair is present per series
infix_parser(typing, bitshift, type, LEX_TYPE_COLON, true)

// bitand: bitshift (('&') bitshift)*
infix_parser(bitand, typing, type, LEX_TYPE_AMP, true)

// bitor: bitand (('|') bitand)*
infix_parser(bitor, bitand, type, LEX_TYPE_BAR, true)

// bitxor: bitor (('^') bitor)*
infix_parser(bitxor, bitor, type, LEX_TYPE_CARET, true)

// compare: bitxor (/[=!]=|[><]=?|in/ bitxor)*
infix_parser(compare, bitxor, type, LEX_TYPE_COMPARE, true)

// and: compare (('and') compare)*
infix_parser(and, compare, type, LEX_TYPE_AND, true)

// or: and (('or') and)*
infix_parser(or, and, type, LEX_TYPE_OR, true)

// xor: or (('xor') or)*
infix_parser(xor, or, type, LEX_TYPE_XOR, true)

// assignment: symbol '..' symbol '=' expression
//           | symbol ':' type_func '=' expression
//           | symbol ('.' value)+ '=' expression
//           | symbol (operand | symbol ':' (symbol | type_func))* '=' expression
//			 | symbol '=' 'type' type_func
//			 | symbol '=' 'enum' enum_parser
//			 | symbol '=' 'class' class
//			 | symbol '=' expression
parse_result_t assignment(lexer_t* lex);

// with_expr: 'with' (assignment ',')+ expression
parse_result_t with_expr(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume with keyword
	consume(with, true, string, lex, "with", (parse_result_t) {false}, false);

	// Consume one assignment and add it to the with keyword ast node
	call(assign, true, assignment, lex, with, true);
	list_append_element(with.ast->children, with.ast->children_size, with.ast->children_count, ast_t*, assign.ast);

	// Consume a comma
	consume(comma, true, type, lex, LEX_TYPE_COMMA, with, true);
	clean_parse_result(comma);

	// Consume a newline
	repush_lexer(lex);
	consume(newline, false, type, lex, LEX_TYPE_NEWLINE, with, false);
	clean_parse_result(newline);

	while (true)
	{
		push_lexer(lex);

		// Consume an assignment and add it to the with keyword ast node if found
		call(assign, false, assignment, lex, with, false);
		if (!assign.succ)
		{
			clean_parse_result(assign);
			break;
		}
		list_append_element(with.ast->children, with.ast->children_size, with.ast->children_count, ast_t*, assign.ast);

		// Consume a comma
		consume(comma, true, type, lex, LEX_TYPE_COMMA, with, true);
		clean_parse_result(comma);

		// Consume a newline
		repush_lexer(lex);
		consume(newline, false, type, lex, LEX_TYPE_NEWLINE, with, false);
		clean_parse_result(newline);
	}

	// Get an expression
	call(expr, true, expression, lex, with, true);
	list_append_element(with.ast->children, with.ast->children_size, with.ast->children_count, ast_t*, expr.ast);
	return with;
}

// match: 'match' expression 'to' value '=>' bitxor ('or' value '=>' bitxor)* ('else' bitxor)?
parse_result_t match(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume the initial expression and form the tree
	consume(match, true, string, lex, "match", (parse_result_t) {false}, false);
	call(expr, true, expression, lex, match, true);
	list_append_element(match.ast->children, match.ast->children_size, match.ast->children_count, ast_t*, expr.ast);

	// Consume to
	repush_lexer(lex);
	consume(newline, false, type, lex, LEX_TYPE_NEWLINE, match, false);
	clean_parse_result(newline);
	consume(to, true, string, lex, "to", match, true);
	repush_lexer(lex);
	consume(newline1, false, type, lex, LEX_TYPE_NEWLINE, match, false);
	clean_parse_result(newline1);

	// Consume first match
	call(val, true, value, lex, match, true);
	list_append_element(match.ast->children, match.ast->children_size, match.ast->children_count, ast_t*, val.ast);
	consume(arrow, true, type, lex, LEX_TYPE_THICC_ARROW, match, true);

	// Form tree for arrow
	arrow.ast->children_size = 2;
	arrow.ast->children = calloc(2, sizeof(ast_t*));
	list_append_element(arrow.ast->children, arrow.ast->children_size, arrow.ast->children_count, ast_t*, val.ast);
	match.ast->children[match.ast->children_count - 1] = arrow.ast;

	// Consume first value
	call(expr1, true, bitxor, lex, match, true);
	list_append_element(arrow.ast->children, arrow.ast->children_size, arrow.ast->children_count, ast_t*, expr1.ast);

	// Consume more matches
	while (true)
	{
		// Push lexer
		push_lexer(lex);
		consume(newline, false, type, lex, LEX_TYPE_NEWLINE, match, false);
		repush_lexer(lex);

		// Consume or
		consume(or, false, string, lex, "or", match, false);
		clean_parse_result(or);
		if (!or.succ) break;

		// Consume match
		call(val, true, value, lex, match, true);
		list_append_element(match.ast->children, match.ast->children_size, match.ast->children_count, ast_t*, val.ast);
		consume(arrow, true, type, lex, LEX_TYPE_THICC_ARROW, match, true);

		// Form tree for arrow
		arrow.ast->children_size = 2;
		arrow.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(arrow.ast->children, arrow.ast->children_size, arrow.ast->children_count, ast_t*, val.ast);
		match.ast->children[match.ast->children_count - 1] = arrow.ast;

		// Consume value
		call(expr, true, bitxor, lex, match, true);
		list_append_element(arrow.ast->children, arrow.ast->children_size, arrow.ast->children_count, ast_t*, expr.ast);
	}

	// Consume else
	repush_lexer(lex);
	consume(newline2, false, type, lex, LEX_TYPE_NEWLINE, match, false);
	consume(elsey, false, string, lex, "else", match, false);
	if (elsey.succ)
	{
		// Form tree for else
		elsey.ast->children_size = 1;
		elsey.ast->children = calloc(1, sizeof(ast_t*));
		list_append_element(match.ast->children, match.ast->children_size, match.ast->children_count, ast_t*, elsey.ast);

		// Consume value
		call(expr, true, bitxor, lex, match, true);
		list_append_element(elsey.ast->children, elsey.ast->children_size, elsey.ast->children_count, ast_t*, expr.ast);
	} else clean_parse_result(elsey);
	return match;
}

// if_expr: 'if' expression 'then' expression ('else' expression)?
parse_result_t if_expr(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume an if condition and form the tree
	consume(iffy, true, string, lex, "if", (parse_result_t) {false}, false);
	call(cond, true, expression, lex, iffy, true);
	iffy.ast->children_size = 3;
	iffy.ast->children = calloc(3, sizeof(ast_t*));
	list_append_element(iffy.ast->children, iffy.ast->children_size, iffy.ast->children_count, ast_t*, cond.ast);

	// Consume a newline
	repush_lexer(lex);
	consume(newline, false, type, lex, LEX_TYPE_NEWLINE, iffy, false);
	clean_parse_result(newline);

	// Consume then
	consume(then, true, string, lex, "then", iffy, true);
	clean_parse_result(then);

	// Consume a newline
	repush_lexer(lex);
	consume(newline2, false, type, lex, LEX_TYPE_NEWLINE, iffy, false);
	clean_parse_result(newline2);

	// Consume body
	call(body, true, statement, lex, iffy, true);
	list_append_element(iffy.ast->children, iffy.ast->children_size, iffy.ast->children_count, ast_t*, body.ast);

	// Consume a newline
	repush_lexer(lex);
	consume(newline3, false, type, lex, LEX_TYPE_NEWLINE, iffy, false);
	clean_parse_result(newline3);

	// Consume else
	consume(elsy, true, string, lex, "else", iffy, true);

	// Consume a newline
	repush_lexer(lex);
	consume(newline4, false, type, lex, LEX_TYPE_NEWLINE, iffy, false);
	clean_parse_result(newline4);

	// Consume else body
	call(else_body, true, statement, lex, iffy, true);
	list_append_element(iffy.ast->children, iffy.ast->children_size, iffy.ast->children_count, ast_t*, else_body.ast);
	return iffy;
}

// type_func: type_parameterised ('|' type_parameterised)*
parse_result_t type_func(lexer_t* lex);

// type_typed: symbol ':' type_union
parse_result_t type_typed(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume the symbol
	consume(symbol, true, type, lex, LEX_TYPE_SYMBOL, (parse_result_t) {false}, false);

	// Consume a colon
	consume(colon, true, type, lex, LEX_TYPE_COLON, symbol, false);

	// Create tree
	colon.ast->children_size = 2;
	colon.ast->children = calloc(2, sizeof(ast_t*));
	list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, symbol.ast);

	// Colon must be followed by a type
	repush_lexer(lex);
	consume(type_sym, false, type, lex, LEX_TYPE_SYMBOL, colon, false);
	if (type_sym.succ)
	{
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, type_sym.ast);
		return colon;
	} else clean_parse_result(type_sym);

	consume(lparen, true, string, lex, "(", colon, true);
	clean_parse_result(lparen);
	call(type, true, type_func, lex, colon, true);
	list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, type.ast);
	consume(rparen, true, string, lex, ")", colon, true);
	clean_parse_result(rparen);
	return colon;
}

// type_paren: '(' type_func ')'
parse_result_t type_paren(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume parenthesised type
	consume(lparen, true, string, lex, "(", (parse_result_t) {false}, false);
	clean_parse_result(lparen);
	call(subtype, true, type_func, lex, (parse_result_t) {false}, true);
	consume(rparen, true, string, lex, ")", subtype, true);
	clean_parse_result(rparen);
	return subtype;
}

// type_application: symbol (symbol | type_paren)*
parse_result_t type_application(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume the type
	consume(type, true, type, lex, LEX_TYPE_SYMBOL, (parse_result_t) {false}, false);

	// Consume any arguments if necessary
	while (true)
	{
		// Push lexer
		push_lexer(lex);

		// Get argument
		consume(arg, false, type, lex, LEX_TYPE_SYMBOL, type, false);
		if (!arg.succ)
		{
			clean_parse_result(arg);

			// Get parenthesised type
			call(paren, false, type_paren, lex, type, false);
			if (paren.succ)
				arg = paren;
			else
			{
				clean_parse_result(paren);
				break;
			}
		}

		// Construct tree
		list_append_element(type.ast->children, type.ast->children_size, type.ast->children_count, ast_t*, arg.ast);
	}

	return type;
}

// type_intersect_arg: typed_type | type_application
parse_result_t type_intersect_arg(lexer_t* lex)
{
	// Push lexer
	push_lexer(lex);

	// Try to consume a typed result
	call(typed, false, type_typed, lex, (parse_result_t) {false}, false);
	if (typed.succ)
		return typed;
	else clean_parse_result(typed);

	// Consume a type application
	call(app, false, type_application, lex, (parse_result_t) {false}, false);
	if (app.succ)
		return app;
	else clean_parse_result(app);

	// Consume a parenthesised type
	call(subtype, false, type_paren, lex, (parse_result_t) {false}, false);
	return subtype;
}

// type_intersection: type_intersect_arg ('&' type_intersect_arg)*
parse_result_t type_intersection(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume first argument
	call(arg, true, type_intersect_arg, lex, (parse_result_t) {false}, false);
	parse_result_t top = arg;
	bool second = true;

	// Consume the rest of the arguments
	while (true)
	{
		push_lexer(lex);

		// Consume the operator
		consume(op, false, type, lex, LEX_TYPE_AMP, top, false);
		if (!op.succ)
		{
			clean_parse_result(op);
			break;
		} else if (second)
		{
			list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t*, top.ast);
			top = op;
			second = false;
		} else clean_parse_result(op);

		// Consume a new item
		call(arg, true, type_intersect_arg, lex, (parse_result_t) {false}, false);
		list_append_element(top.ast->children, top.ast->children_size, top.ast->children_count, ast_t*, arg.ast);
	}

	return top;
}

// type_product: type_intersection ('*' type_intersection)*
parse_result_t type_product(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume first argument
	call(arg, true, type_intersection, lex, (parse_result_t) {false}, false);
	parse_result_t top = arg;
	bool second = true;

	// Consume the rest of the arguments
	while (true)
	{
		push_lexer(lex);

		// Consume the operator
		consume(op, false, string, lex, "*", top, false);
		if (!op.succ)
		{
			clean_parse_result(op);
			break;
		} else if (second)
		{
			list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t*, top.ast);
			top = op;
			second = false;
		} else clean_parse_result(op);

		// Consume a new item
		call(arg, true, type_intersection, lex, (parse_result_t) {false}, false);
		list_append_element(top.ast->children, top.ast->children_size, top.ast->children_count, ast_t*, arg.ast);
	}

	return top;
}

// type_union: type_product ('|' type_product)*
parse_result_t type_union(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume first argument
	call(arg, true, type_product, lex, (parse_result_t) {false}, false);
	parse_result_t top = arg;
	bool second = true;

	// Consume the rest of the arguments
	while (true)
	{
		push_lexer(lex);

		// Consume the operator
		consume(op, false, type, lex, LEX_TYPE_BAR, top, false);
		if (!op.succ)
		{
			clean_parse_result(op);
			break;
		} else if (second)
		{
			list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t*, top.ast);
			top = op;
			second = false;
		} else clean_parse_result(op);

		// Consume a new item
		call(arg, true, type_product, lex, (parse_result_t) {false}, false);
		list_append_element(top.ast->children, top.ast->children_size, top.ast->children_count, ast_t*, arg.ast);
	}

	return top;
}

// type_parameterised: type_union ('=>' type_union)*
parse_result_t type_parameterised(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume parameters
	parse_result_t top = {false};
	bool first = true;
	while (true)
	{
		push_lexer(lex);

		// Consume parameter
		consume(param, false, type, lex, LEX_TYPE_SYMBOL, top, false);
		if (param.succ && first)
			top = param;
		else if (!param.succ)
		{
			clean_parse_result(param);
			break;
		} else if (!first)
			list_append_element(top.ast->children, top.ast->children_size, top.ast->children_count, ast_t*, param.ast);

		// Consume the operator
		consume(op, false, type, lex, LEX_TYPE_THICC_ARROW, top, false);
		if (!op.succ)
		{
			clean_parse_result(op);
			if (!first)
				top.ast->children[--top.ast->children_count] = NULL;
			break;
		} else if (first)
		{
			list_append_element(op.ast->children, op.ast->children_size, op.ast->children_count, ast_t*, top.ast);
			top = op;
			first = false;
		} else clean_parse_result(op);
	}

	// Consume type
	call(arg, true, type_union, lex, top, false);
	if (first)
		top = arg;
	else list_append_element(top.ast->children, top.ast->children_size, top.ast->children_count, ast_t*, arg.ast);
	return top;
}

// type_func: type_parameterised ('->' type_parameterised)*
infix_parser(type_func, type_parameterised, type, LEX_TYPE_RIGHT_ARROW, false)

// enum_parser: 'enum' symbol ('|' symbol)*
parse_result_t enum_parser(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume enum keyword
	consume(top, true, string, lex, "enum", (parse_result_t) {false}, false);

	// Consume first symbol
	consume(item, true, type, lex, LEX_TYPE_SYMBOL, top, true);
	list_append_element(top.ast->children, top.ast->children_size, top.ast->children_count, ast_t*, item.ast);

	// Consume the rest of the symbols
	while (true)
	{
		push_lexer(lex);

		// Consume a bar
		consume(bar, false, type, lex, LEX_TYPE_BAR, top, false);
		clean_parse_result(bar);
		if (!bar.succ) break;

		// Consume a new item
		consume(item, true, type, lex, LEX_TYPE_SYMBOL, top, true);
		list_append_element(top.ast->children, top.ast->children_size, top.ast->children_count, ast_t*, item.ast);
	}

	return top;
}

// class: 'class' symbol* 'where' ('true' | class_comp ('and' class_comp)*)
parse_result_t class(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume class keyword
	consume(top, true, string, lex, "class", (parse_result_t) {false}, false);

	// Consume type class name
	consume(symbol, true, type, lex, LEX_TYPE_SYMBOL, top, true);
	list_append_element(top.ast->children, top.ast->children_size, top.ast->children_count, ast_t*, symbol.ast);

	// Consume parameters to type class
	while (true)
	{
		push_lexer(lex);
		consume(arg, false, type, lex, LEX_TYPE_SYMBOL, top, false);
		if (!arg.succ)
		{
			clean_parse_result(arg);
			break;
		}

		// Add parameter to symbol
		list_append_element(symbol.ast->children, symbol.ast->children_size, symbol.ast->children_count, ast_t*, arg.ast);
	}

	// Consume where
	consume(where, true, string, lex, "where", top, true);
	clean_parse_result(where);

	// Consume true
	// TODO: More type class stuff
	consume(truthy, true, string, lex, "true", top, true);
	list_append_element(top.ast->children, top.ast->children_size, top.ast->children_count, ast_t*, truthy.ast);
	return top;
}

// assignment: symbol '..' symbol '=' expression
//           | symbol ':' type_func '=' expression
//           | symbol ('.' value)+ '=' expression
//           | symbol (operand | symbol ':' (symbol | type_func))* '=' expression
//			 | symbol '=' 'type' type_func
//			 | symbol '=' 'enum' enum_parser
//			 | symbol '=' 'class' class
//			 | symbol '=' expression
parse_result_t assignment(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume a symbol (there must be at least one for an assignment)
	consume(symbol, true, type, lex, LEX_TYPE_SYMBOL, (parse_result_t) {false}, false);
	repush_lexer(lex);

	// Try to consume a range operator
	consume(range, false, type, lex, LEX_TYPE_RANGE, symbol, false);
	if (range.succ)
	{
		// Add the head to the range operator ast node
		range.ast->children_size = 2;
		range.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(range.ast->children, range.ast->children_size, range.ast->children_count, ast_t*, symbol.ast);

		// Consume another symbol and add it as the tail to the range operator ast node
		consume(tail, true, type, lex, LEX_TYPE_SYMBOL, range, true);
		list_append_element(range.ast->children, range.ast->children_size, range.ast->children_count, ast_t*, tail.ast);

		// Consume equal sign
		consume(assign, true, type, lex, LEX_TYPE_ASSIGN, range, true);
		assign.ast->children_size = 2;
		assign.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, range.ast);

		// Get expression
		call(expr, true, expression, lex, assign, true);

		// Add the expression to the assignment operator ast node
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, expr.ast);
		return assign;
	}

	// Try to consume a colon
	clean_parse_result(range);
	consume(colon, false, type, lex, LEX_TYPE_COLON, symbol, false);
	if (colon.succ)
	{
		// Add the variable name to the type operator ast node
		colon.ast->children_size = 2;
		colon.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, symbol.ast);

		// Consume the type
		call(type, true, type_func, lex, colon, true);
		list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, type.ast);
		push_lexer(lex);

		// Consume equal sign (it's optional)
		consume(assign, false, type, lex, LEX_TYPE_ASSIGN, colon, false);
		if (!assign.succ)
		{
			clean_parse_result(assign);
			return colon;
		}

		// Create the tree
		assign.ast->children_size = 2;
		assign.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, colon.ast);

		// Get expression
		call(expr, true, expression, lex, assign, true);

		// Add the expression to the assignment operator ast node
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, expr.ast);
		return assign;
	}

	// Try to consume a dot
	clean_parse_result(colon);
	consume(dot, false, type, lex, LEX_TYPE_DOT, symbol, false);
	if (dot.succ)
	{
		// Add the parent name to the type operator ast node
		list_append_element(dot.ast->children, dot.ast->children_size, dot.ast->children_count, ast_t*, symbol.ast);

		// Consume a value
		call(val, true, value, lex, dot, true);
		list_append_element(dot.ast->children, dot.ast->children_size, dot.ast->children_count, ast_t*, val.ast);

		while (true)
		{
			// Push the lexer
			push_lexer(lex);

			// Consume a dot
			consume(dot2, false, type, lex, LEX_TYPE_DOT, dot, false);
			clean_parse_result(dot2);
			if (!dot2.succ) break;

			// Consume a value
			call(val, true, value, lex, dot, true);
			list_append_element(dot.ast->children, dot.ast->children_size, dot.ast->children_count, ast_t*, val.ast);
		}

		// Consume equal sign
		consume(assign, true, type, lex, LEX_TYPE_ASSIGN, dot, false);
		assign.ast->children_size = 2;
		assign.ast->children = calloc(2, sizeof(ast_t*));
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, dot.ast);

		// Get expression
		call(expr, true, expression, lex, assign, true);

		// Add the expression to the assignment operator ast node
		list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, expr.ast);
		return assign;
	}

	// Try to consume arguments
	clean_parse_result(dot);
	while (true)
	{
		// Push the lexer
		push_lexer(lex);

		// Consume an operand and add it to the symbol ast node
		consume(arg, false, tag, lex, LEX_TAG_OPERAND, symbol, false);
		if (!arg.succ)
		{
			clean_parse_result(arg);
			break;
		}
		list_append_element(symbol.ast->children, symbol.ast->children_size, symbol.ast->children_count, ast_t*, arg.ast);

		// If a symbol was absorbed, attach a type
		if (arg.ast->value.type == LEX_TYPE_SYMBOL)
		{
			// Consume a colon if able
			push_lexer(lex);
			consume(colon, false, type, lex, LEX_TYPE_COLON, symbol, false);
			if (!colon.succ)
			{
				clean_parse_result(colon);
				continue;
			}

			// Add the symbol node to the colon
			colon.ast->children_size = 2;
			colon.ast->children = calloc(2, sizeof(ast_t*));
			list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, symbol.ast->children[symbol.ast->children_count - 1]);
			symbol.ast->children[symbol.ast->children_count - 1] = colon.ast;

			// Consume the type and add it to the colon ast node
			repush_lexer(lex);
			consume(lparen, false, string, lex, "(", symbol, false);
			clean_parse_result(lparen);
			if (lparen.succ)
			{
				call(type, true, type_func, lex, symbol, true);
				consume(rparen, true, string, lex, ")", symbol, true);
				list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, type.ast);
			} else
			{
				consume(type, true, type, lex, LEX_TYPE_SYMBOL, symbol, true);
				list_append_element(colon.ast->children, colon.ast->children_size, colon.ast->children_count, ast_t*, type.ast);

				// Consume attribute if applicable
				push_lexer(lex);
				consume(dot, false, type, lex, LEX_TYPE_DOT, symbol, false);
				if (dot.succ)
				{
					dot.ast->children_count = 2;
					dot.ast->children = calloc(2, sizeof(ast_t*));
					list_append_element(dot.ast->children, dot.ast->children_size, dot.ast->children_count, ast_t*, type.ast);
					colon.ast->children[1] = dot.ast;
					consume(attr, true, type, lex, LEX_TYPE_SYMBOL, symbol, true);
					list_append_element(dot.ast->children, dot.ast->children_size, dot.ast->children_count, ast_t*, attr.ast);
				} else clean_parse_result(dot);
			}
		}
	}

	// Consume equal sign
	consume(assign, true, type, lex, LEX_TYPE_ASSIGN, symbol, false);
	repush_lexer(lex);
	assign.ast->children_size = 2;
	assign.ast->children = calloc(2, sizeof(ast_t*));
	list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, symbol.ast);

	if (symbol.ast->children_count == 0)
	{
		// Try to consume type keyword
		consume(type_keyword, false, string, lex, "type", assign, false);
		if (type_keyword.succ)
		{
			// Get the type
			list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, type_keyword.ast);
			call(type, true, type_func, lex, assign, true);

			// Build the tree
			type_keyword.ast->children = calloc(1, sizeof(ast_t*));
			list_append_element(type_keyword.ast->children, type_keyword.ast->children_size, type_keyword.ast->children_count, ast_t*, type.ast);
			return assign;
		} else clean_parse_result(type_keyword);

		// Try to consume enum
		call(enumy, false, enum_parser, lex, assign, false);
		if (enumy.succ)
		{
			list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, enumy.ast);
			return assign;
		} else clean_parse_result(enumy);

		// Try to consume class
		call(classy, false, class, lex, assign, false);
		if (classy.succ)
		{
			list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, classy.ast);
			return assign;
		} else clean_parse_result(classy);
	}

	// Get expression
	call(expr, true, expression, lex, assign, true);

	// Add the expression to the assignment operator ast node
	list_append_element(assign.ast->children, assign.ast->children_size, assign.ast->children_count, ast_t*, expr.ast);
	return assign;
}


// for_loop: 'for' symbol 'in' expression expression
parse_result_t for_loop(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume for
	consume(fory, true, string, lex, "for", (parse_result_t) {false}, false);

	{
		// Push the lexer
		push_lexer(lex);

		// Check for a quantifier
		consume(all, false, string, lex, "all", fory, false);
		if (all.succ)
			list_append_element(fory.ast->children, fory.ast->children_size, fory.ast->children_count, ast_t*, all.ast);
		else
		{
			// stan loona
			clean_parse_result(all);
			consume(some, false, string, lex, "some", fory, false);
			if (some.succ)
				list_append_element(fory.ast->children, fory.ast->children_size, fory.ast->children_count, ast_t*, some.ast);
		}
	}

	// Consume the variable name
	consume(symbol, true, type, lex, LEX_TYPE_SYMBOL, fory, true);
	list_append_element(fory.ast->children, fory.ast->children_size, fory.ast->children_count, ast_t*, symbol.ast);

	// Consume the iterator
	consume(in, true, string, lex, "in", fory, true);
	clean_parse_result(in);
	call(iter, true, value, lex, fory, true);
	list_append_element(fory.ast->children, fory.ast->children_size, fory.ast->children_count, ast_t*, iter.ast);

	// Consume a newline
	repush_lexer(lex);
	consume(newline, false, type, lex, LEX_TYPE_NEWLINE, fory, false);
	clean_parse_result(newline);

	// Consume the body
	call(body, true, statement, lex, fory, true);
	list_append_element(fory.ast->children, fory.ast->children_size, fory.ast->children_count, ast_t*, body.ast);
	return fory;
}

// where_expr: symbol 'in' expression 'where' expression
parse_result_t where_expr(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume a symbol
	consume(symbol, true, type, lex, LEX_TYPE_SYMBOL, (parse_result_t) {false}, false);

	// Consume the in operator and form the tree
	consume(in, true, string, lex, "in", symbol, false);
	in.ast->children_size = 2;
	in.ast->children = calloc(2, sizeof(ast_t*));
	list_append_element(in.ast->children, in.ast->children_size, in.ast->children_count, ast_t*, symbol.ast);

	// Consume the iterator
	call(iter, true, expression, lex, in, false);
	list_append_element(in.ast->children, in.ast->children_size, in.ast->children_count, ast_t*, iter.ast);

	// Consume the where operator and form the tree
	consume(where, true, string, lex, "where", in, false);
	where.ast->children_size = 2;
	where.ast->children = calloc(2, sizeof(ast_t*));
	list_append_element(where.ast->children, where.ast->children_size, where.ast->children_count, ast_t*, in.ast);

	// Consume a newline
	repush_lexer(lex);
	consume(newline, false, type, lex, LEX_TYPE_NEWLINE, where, false);
	clean_parse_result(newline);

	// Consume the predicate
	call(pred, true, expression, lex, in, true);
	list_append_element(where.ast->children, where.ast->children_size, where.ast->children_count, ast_t*, pred.ast);
	return where;
}

// expression: 'pass' | 'stop' | with_expr | if_expr | for_loop | xor
parse_result_t expression(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Consume pass
	consume(pass, false, string, lex, "pass", (parse_result_t) {false}, false);
	if (pass.succ)
		return pass;

	// Consume stop
	clean_parse_result(pass);
	consume(stop, false, string, lex, "stop", (parse_result_t) {false}, false);
	if (stop.succ)
		return stop;

	// Call where expression
	clean_parse_result(stop);
	call(where, false, where_expr, lex, (parse_result_t) {false}, false);
	if (where.succ)
		return where;

	// Call with expression if where expression is not applicable
	clean_parse_result(where);
	call(with, false, with_expr, lex, (parse_result_t) {false}, false);
	if (with.succ)
		return with;

	// Call if expression if with expression is not applicable
	clean_parse_result(with);
	call(iffy, false, if_expr, lex, (parse_result_t) {false}, false);
	if (iffy.succ)
		return iffy;

	// Call for loop if if expression is not applicable
	clean_parse_result(iffy);
	call(fory, false, for_loop, lex, (parse_result_t) {false}, false);
	if (fory.succ)
		return fory;

	// Call pattern matching if for loop is not applicable
	clean_parse_result(fory);
	call(matchy, false, match, lex, (parse_result_t) {false}, false);
	if (matchy.succ)
		return matchy;

	// Call xor if pattern matching is not applicable
	clean_parse_result(matchy);
	return xor(lex);
}

// statement: assignment | expression
parse_result_t statement(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);

	// Call assignment
	call(assign, false, assignment, lex, (parse_result_t) {false}, false);
	if (assign.succ)
		return assign;

	// Call expression if assignment is not applicable
	clean_parse_result(assign);
	call(expr, false, expression, lex, (parse_result_t) {false}, false);
	return expr;
}

// lang_parser(lexer_t*) -> parse_result_t
// Parses the curly language.
// lang_parser: statement*
parse_result_t lang_parser(lexer_t* lex)
{
	// Push the lexer
	push_lexer(lex);
	parse_result_t result = succ_result(init_ast((token_t) {0, 0, NULL, 0, 0, 0}));

	while (true)
	{
		// Push the lexer
		push_lexer(lex);

		// Consume one statement
		call(state, false, statement, lex, result, false);
		if (!state.succ)
			clean_parse_result(state);
		else list_append_element(result.ast->children, result.ast->children_size, result.ast->children_count, ast_t*, state.ast);

		// Consume a newline
		repush_lexer(lex);
		consume(newline, false, type, lex, LEX_TYPE_NEWLINE, result, false);
		if (!newline.succ)
		{
			clean_parse_result(newline);
			break;
		}
	}

	// Assert that the end of file has been reached
	consume(eof, true, type, lex, LEX_TYPE_EOF, result, true);
	clean_parse_result(eof);
	return result;
}

#undef infix_parser
#undef repush_lexer
#undef push_lexer
#undef call
#undef consume

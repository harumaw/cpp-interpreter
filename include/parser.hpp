#pragma once

#include <unordered_map>
#include <unordered_set>

#include "token.hpp"
#include "ast.hpp"
#include "declaration.hpp"
#include "statement.hpp"
#include "expression.hpp"

class Parser {
private:
	
	std::vector<Token> tokens;
	std::size_t offset;
	
	static const std::unordered_set<std::string> unary_operators;

public:
	Parser(const std::vector<Token>&);

	bool is_type_specifier();
public:
	std::shared_ptr<TranslationUnit> parse();

	declaration parse_declaration();
	func_declaration parse_function_declaration();
	parameter_declaration parse_parameter_declaration();
	var_declaration parse_var_declaration();
	struct_declaration parse_struct_declaration();
	array_declaration parse_array_declaration();
	init_declarator parse_init_declarator();
	declarator parse_declarator();
public:
	statement parse_statement();
	compound_statement parse_compound_statement();
	conditional_statement parse_conditional_statement();
	loop_statement parse_loop_statement();
	while_statement parse_while_statement();
	for_statement parse_for_statement();
	jump_statement parse_jump_statement();
	break_statement parse_break_statement();
	continue_statement parse_continue_statement();
	return_statement parse_return_statement();
	declaration_statement parse_declaration_statement();
	expression_statement parse_expression_statement();
	do_while_statement parse_do_while_statement();
public:
	expression parse_expression();
	expression parse_comma_expression();
	expression parse_assignment();
	expression parse_ternary_expression();
	expression parse_logical_or_expression();
	expression parse_logical_and_expression();
	expression parse_equality_expression();	
	expression parse_compared_expression();

	expression parse_sum_expression();
	expression parse_mul_expression();
	expression parse_pow_expression();	
	expression parse_unary_expression();
	expression parse_postfix_expression();
	expression parse_base();

/*
	expression parse_expression();
	assignment_expression parse_assignment();
	ternary_expression parse_ternary();
	conditional_expression parse_conditional();
	arithmetic_parse parse_arithmetic_expression(int);
	//parse sum, mul, add...
	parse_base();
	a+(b*c)+c;

	a + (b*c) + c

	++a; - unary
	a++ - postfix;
	base->parse_base()-> parse_postfix() -> parse_unary()


*/


/*parse_expression

    parse_assignment - dobavit
    /parse_ternary
    /parse_compared - delitsya na svoi podtipy
    1 -/parse_sum
    /parse_mul
    /parse_pow
    /parse_unary
    /parse_postfix
    /parse_base


*/
private:
	template<typename... Args>
	bool check_token(const Args&...);

	template<typename... Args>
	bool match_token(const Args&...);

	template<typename... Args>
	std::string extract_token(const Args&...);

	template<typename... Args>
	bool match_pattern(const Args&...);


	Token peek_token(int lookahead);

};

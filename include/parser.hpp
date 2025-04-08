#pragma once

#include <unordered_map>
#include <unordered_set>

#include "token.hpp"
#include "ast.hpp"
#include "declaration.hpp"
#include "statement.hpp"
#include "expression.hpp"

class Parser {
public:
	Parser(const std::vector<Token>&);

	std::shared_ptr<RootNode> parse();
	std::shared_ptr<RootNode> parse_root();
	declaration parse_declaration();
	func_declaration parse_function_declaration();
	parameter_declaration parse_parameter_declaration();
	var_declaration parse_var_declaration();
	struct_declaration parse_struct_declaration();
	init_declarator parse_init_declarator();
	declarator parse_declarator();


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


	expression parse_expression();
	expression parse_member_access(std::shared_ptr<Expression>);
	binary_expression parse_binary_expression(int);
	unary_expression parse_unary_expression();
	postfix_expression parse_postfix_expression();
	func_param parse_function_call_expression();
	expression parse_subscript_expression();
	primary_expression parse_primary_expression();
	parentsized_expression parse_parenthesized_expression();

private:
	std::vector<Token> tokens;
	std::size_t offset;

private:
	template<typename... Args>
	bool check_token(const Args&...);

	template<typename... Args>
	bool match_token(const Args&...);

	template<typename... Args>
	std::string extract_token(const Args&...);

	template<typename... Args>
	bool match_pattern(const Args&...);

private:
	static const std::unordered_map<std::string, int> operator_precedences;
	static const std::unordered_set<std::string> unary_operators;
};

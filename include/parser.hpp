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

	std::shared_ptr<TranslationUnit> parse();
	std::shared_ptr<Declaration> parse_declaration();
	std::shared_ptr<FuncDeclaration> parse_function_declaration();
	std::shared_ptr<ParameterDeclaration> parse_parameter_declaration();
	std::shared_ptr<VarDeclaration> parse_var_declaration();
	std::shared_ptr<Declaration::InitDeclarator> parse_init_declarator();
	std::shared_ptr<Declaration::Declarator> parse_declarator();

	std::shared_ptr<Statement> parse_statement();
	std::shared_ptr<CompoundStatement> parse_compound_statement();
	std::shared_ptr<ConditionalStatement> parse_conditional_statement();
	std::shared_ptr<LoopStatement> parse_loop_statement();
	std::shared_ptr<WhileStatement> parse_while_statement();
	std::shared_ptr<ForStatement> parse_for_statement();
	std::shared_ptr<RepeatStatement> parse_repeat_statement();
	std::shared_ptr<JumpStatement> parse_jump_statement();
	std::shared_ptr<BreakStatement> parse_break_statement();
	std::shared_ptr<ContinueStatement> parse_continue_statement();
	std::shared_ptr<ReturnStatement> parse_return_statement();
	std::shared_ptr<DeclarationStatement> parse_declaration_statement();
	std::shared_ptr<ExpressionStatement> parse_expression_statement();

	std::shared_ptr<Expression> parse_expression();
	std::shared_ptr<BinaryExpression> parse_binary_expression(int);
	std::shared_ptr<UnaryExpression> parse_unary_expression();
	std::shared_ptr<PostfixExpression> parse_postfix_expression();
	std::vector<std::shared_ptr<Expression>> parse_function_call_expression();
	std::shared_ptr<Expression> parse_subscript_expression();
	std::shared_ptr<PrimaryExpression> parse_primary_expression();
	std::shared_ptr<ParenthesizedExpression> parse_parenthesized_expression();

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

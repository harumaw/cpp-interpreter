#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "token.hpp"
#include "ast.hpp"


class Parser {
public:
	Parser(const std::vector<Token>&);
	std::vector<node> parse();
private:
	std::vector<Token> tokens;
	std::size_t offset;
	
	static const std::unordered_map<std::string, int> num_operators;
	static const std::unordered_set<std::string> types;
    static const std::unordered_set<std::string> keywords;
    static const std::unordered_set<std::string> operators;
	static const std::vector<std::string> unary_operators;
			

	decl parse_variable_declaration();
	decl parse_function_declaration();
	
	
	st parse_if_statement();
	st parse_else_statement();
	st parse_conditional();
	st parse_while();
    st parse_for();
	st parse_loop();
	st parse_block_statement();
	st parse_jump();
	

	expr binary_parse();
	expr parse_binary_expression(int);
	
	expr parse_prefix();
	expr parse_postfix();
	expr parse_unary();
	
    expr parse_function_call();
    std::vector<expr> parse_function_interior();
    	
    expr parse_parenthesized_expression();
    expr parse_string();
    	
    	
	node parse_token();
	node parse_base();
    	    	
    

	bool is_global_scope() const;
	bool match_value(std::string value) const;
	bool match_type(TokenType expected_type) const;
    std::string value_extract(std::string value);
    std::string type_extract(TokenType expected_type);
    std::string getTypeFrom_string(const std::string&);
};
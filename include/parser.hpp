#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>


#include "token.hpp"
#include "ast.hpp"


class Parser{
    public:
        Parser(const std::vector<Token>&);
        std::vector<ast_node> parse();
    
    private:
        std::vector<Token> tokens;
        std::size_t index = 0;
        
        static const std::unordered_map<TokenType, int> num_opers;
        static const std::vector<std::string> types;
        static const std::vector<std::string> keywords;
        static const std::vector<std::string> operators;
        static const std::vector<std::string> unary_operators;

        declaration parse_variable_declaration();
        declaration parse_function_declaration();

        statement parse_if_statement();
        statement parse_else_statement();
        statement parse_conditional();
        statement parse_while_loop();
        statement parse_for_loop();
        statement parse_loop();
        statement parse_block_statement();
        statement parse_jump();

        expression parse_binary();
        expression parse_binary_expr();

        expression parse_prefix();
        expression parse_postfix();
        expression parse_unary();   

        expression parse_function_call();
        std::vector<expression> parse_function_interior();

        expression parse_parenthesized_expression();
    	expression parse_string();
    	
        ast_node parse_token();
	    ast_node parse_base();
};
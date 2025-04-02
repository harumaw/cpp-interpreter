#pragma once

#include <vector>
#include <unordered_map>
#include <set>
#include "token.hpp"




class Lexer{
    public:
        Lexer(const std::string& filename);
        std::vector<Token> tokenize();

        static void print_tokens(const std::vector<Token>& tokens);

    private:

        std::string filename;
        std::size_t offset;


        static std::set<std::string> types;
        static std::unordered_map<std::string, TokenType> operators;
        static std::unordered_map<std::string, TokenType> punctuators;
        static std::unordered_map<std::string, TokenType> keywords;
        static std::string spec_symbols;
        std::string operator_char = "+-*/%^&|=<>!~";
    
        Token extract_literal(const std::string& line);
        Token extract_type(const std::string& line);
        Token extract_operator(const std::string& line);
        Token extract_punctuator(const std::string& line);
        Token extract_id(const std::string& line);
        Token extract_keyword(const std::string& line);

        
        static std::string token_type_to_string(TokenType type);
};

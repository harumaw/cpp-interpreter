#pragma once

#include <vector>
#include <unordered_set>

#include <token.h>


void print_tokens(std::vector<Token> tokens);

class Lexer{
    public:
        Lexer(const std::string& filename);
        std::vector<Token> tokenize();

    private:

        std::string filename;
        std::size_t offset;


        Token extract_number(std::string&);
        Token extract_identifier(std::string&);
        Token extract_operator(std::string&);
        Token extract_punctuator(std::string&);

        std::string operator_char = "+-*/%^&|=><!";
	    std::string punctuator_char = ",()[]{};\":'";
	
	    std::vector<std::string> keywords = {"if", "elif", "else", "while", "do", "for", "return", "break", "continue"};
	    std::vector<std::string> types = {"int", "float", "double", "char", "string", "bool", "void", "struct"};


    
};
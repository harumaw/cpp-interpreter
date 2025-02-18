#include <iostream>
#include <vector>
#include <string> 
#include <stdexcept>
#include "lexer.hpp"
#include <optional>
#include <fstream> 
#include <unordered_set>

Lexer::Lexer(const std::string& filename) : filename(filename), offset(0) {}


std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    std::ifstream file(filename);

    if (!file){
        std::cerr << "Error opening file" <<filename<< std::endl;
        return tokens;
    }


    std::string line;
    while(std::getline(file, line)){
        offset = 0;
        while(offset < line.size()){
            if(std::isdigit(line[offset])){
                tokens.push_back(extract_number(line));

            } else if(std::isalpha(line[offset]) || line[offset] == '_'){
                tokens.push_back(extract_identifier(line));
            } else if (operator_char.find(line[offset]) != std::string::npos) {
                tokens.push_back(extract_operator(line));
            }
            else if(std::ispunct(line[offset])){
                tokens.push_back(extract_punctuator(line));
            }
            else{
                offset++;
            }
        }
    }
    tokens.push_back({"", TokenType:: END});
    return tokens;

}

Token Lexer::extract_number(std::string& line){
    std::string number;
    std::size_t num_dots = 0;
    while(offset < line.size() && (std::isdigit(line[offset]) || line[offset] == '.')){
        if(line[offset] == '.' && ++num_dots > 1){
            throw std::runtime_error("Too many dots");

        }
        number += line[offset++];

    }
    return {number, TokenType:: NUMBER};

}

Token Lexer::extract_operator(std::string& line){
    std::string op;
    while(offset < line.size() && operator_char.find(line[offset]) != std::string::npos){
        op += line[offset++];

    }
    if (!operator_char.empty()) return {op, TokenType::OPERATOR};
    throw std::runtime_error("Unknown operator: " + op);
}

Token Lexer::extract_punctuator(std::string& line){
    std::string punct;
    while(offset < line.size() && punctuator_char.find(line[offset]) != std::string::npos){
        punct += line[offset++];
        
    }
    if (!punctuator_char.empty()) return {punct, TokenType::PUNCTUATOR};
    throw std::runtime_error("Unknown punct: " + punct);
}

Token Lexer::extract_identifier(std::string& line) {
    std::string identifier;
    while (offset < line.size() && (std::isalnum(line[offset]) || line[offset] == '_')) {
        identifier += line[offset++];
    }
    if (keywords.contains(identifier)) return {identifier, TokenType::KEYWORD};
    if (types.contains(identifier)) return {identifier, TokenType::TYPE}; 
    return {identifier, TokenType::IDENTIFIER}; 
}



std::string token_type_to_string(TokenType type);

void print_tokens(const std::vector<Token>& tokens){
    for(std::size_t i = 0; i < tokens.size(); ++i) {
        std::cout << i << "token " << tokens[i].value << " --> type " << token_type_to_string(tokens[i].type) << std::endl;
    }
}

std::string token_type_to_string(TokenType type){
    switch (type) {
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::OPERATOR: return "OPERATOR";
        case TokenType::KEYWORD: return "KEYWORD";
        case TokenType::PUNCTUATOR: return "PUNCTUATOR";
        case TokenType::TYPE: return "TYPE";
        case TokenType::END: return "END";
        default: return "UNKNOWN";
    }
}


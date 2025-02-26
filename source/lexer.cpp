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
        while (offset < line.size()) {
            char current = line[offset];

            if (std::isdigit(current) || current == '"' || current == '\'' || 
                (current == '-' && offset + 1 < line.size() && std::isdigit(line[offset + 1]))) {
                tokens.push_back(extract_literal(line));
            } else if (std::isalpha(current) || current == '_') {
                tokens.push_back(extract_identifier(line));
            } else if (operator_char.find(current) != std::string::npos) {
                tokens.push_back(extract_operator(line));
            } else if (std::ispunct(current)) {
                tokens.push_back(extract_punctuator(line));
            } else {
                offset++;
            }
        }
    }
    tokens.push_back({"", TokenType::END});
    return tokens;
}

Token Lexer::extract_literal(std::string& line) {
    std::string value;
    bool has_dot = false, has_exp = false;
    

    if (std::isdigit(line[offset])) {  
        while (offset < line.size() && (std::isdigit(line[offset]) || line[offset] == '.' || 
               line[offset] == 'e' || line[offset] == 'E' || line[offset] == '+' || line[offset] == '-')) {
            if (line[offset] == '.') {
                if (has_dot) break; 
                has_dot = true;
            }
            if (line[offset] == 'e' || line[offset] == 'E') {
                if (has_exp) break;
                has_exp = true;
            }
            value += line[offset++];
        }
        return {value, TokenType::LITERAL};
    }

    if (line[offset] == '"') {  
        offset++; 
        while (offset < line.size() && line[offset] != '"') {
            value += line[offset++];
        }
        if (offset < line.size()) {
            offset++;
        } 
        return {value, TokenType::LITERAL};
    }

    if (line[offset] == '\'') { 
        offset++;
        if (offset < line.size() && line[offset + 1] == '\'') {
            value = line[offset];
            offset += 2;
            return {value, TokenType::LITERAL};
        }
    }

    static const std::unordered_set<std::string> keywords_literals = {"true", "false", "nullptr"};
    for (const auto& lit : keywords_literals) {
        if (line.compare(offset, lit.size(), lit) == 0) {
            offset += lit.size();
            return {lit, TokenType::LITERAL};
        }
    }

    throw std::runtime_error("Unknown literal format at: " + line.substr(offset, 10));
}


Token Lexer::extract_operator(std::string& line) {
    std::string op;
   

    if (offset + 1 < line.size() && operator_char.find(line[offset]) != std::string::npos &&
        operator_char.find(line[offset + 1]) != std::string::npos) {
        
        std::string potential_op = {line[offset], line[offset + 1]}; 
        if (valid_ops.find(potential_op) != valid_ops.end()) { 
            offset += 2;
            return {potential_op, TokenType::OPERATOR};
        }
    }

    
    if (operator_char.find(line[offset]) != std::string::npos) {
        op += line[offset++];
        return {op, TokenType::OPERATOR};
    }

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
        case TokenType::LITERAL: return "LITERAl";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::OPERATOR: return "OPERATOR";
        case TokenType::KEYWORD: return "KEYWORD";
        case TokenType::PUNCTUATOR: return "PUNCTUATOR";
        case TokenType::TYPE: return "TYPE";
        case TokenType::END: return "END";
        default: return "UNKNOWN";
    }
}


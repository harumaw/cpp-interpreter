#include "lexer.hpp"
#include <iostream>
#include <fstream>
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& filename) : filename(filename), offset(0) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    std::ifstream file(filename);

    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return tokens;
    }

    std::string line;
    while (std::getline(file, line)) {
        offset = 0;
        while (offset < line.size()) {
            char current = line[offset];

            if (std::isspace(current)) {
                ++offset; // Пропускаем пробелы
                continue;
            }

            if (std::isdigit(current) || current == '"' || current == '\'' || 
                (current == '-' && offset + 1 < line.size() && std::isdigit(line[offset + 1]))) {
                tokens.push_back(extract_literal(line));
            } else if (std::isalpha(current) || current == '_') {
                tokens.push_back(extract_identifier(line));
            } else if (operator_char.find(current) != std::string::npos) {
                tokens.push_back(extract_operator(line));
            } else if (punctuator_char.find(current) != std::string::npos) {
                tokens.push_back(extract_punctuator(line));
            } else {
                std::cerr << "Unknown character: " << current << " at offset " << offset << std::endl;
                ++offset;
            }
        }
    }

    tokens.push_back({"", TokenType::END}); // Добавляем токен конца файла
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

    throw std::runtime_error("Unknown LITERAL format at: " + line.substr(offset, 10));
}

Token Lexer::extract_operator(std::string& line) {
    std::string op;

    // Проверяем двухсимвольные операторы
    if (offset + 1 < line.size()) {
        std::string potential_op = {line[offset], line[offset + 1]};
        if (valid_ops.find(potential_op) != valid_ops.end()) {
            offset += 2;
            return {potential_op, map_operator_to_token_type(potential_op)};
        }
    }

    // Проверяем односимвольные операторы
    if (operator_char.find(line[offset]) != std::string::npos) {
        op += line[offset++];
        if (valid_ops.find(op) != valid_ops.end()) {
            return {op, map_operator_to_token_type(op)};
        }
    }

    throw std::runtime_error("Unknown operator: " + op);
}

Token Lexer::extract_punctuator(std::string& line) {
    std::string punct;
    while (offset < line.size() && punctuator_char.find(line[offset]) != std::string::npos) {
        punct += line[offset++];
    }
    if (!punct.empty()) return {punct, TokenType::PUNCTUATOR};
    throw std::runtime_error("Unknown punctuator: " + punct);
}

Token Lexer::extract_identifier(std::string& line) {
    std::string identifier;
    while (offset < line.size() && (std::isalnum(line[offset]) || line[offset] == '_')) {
        identifier += line[offset++];
    }
    if (keywords.find(identifier) != keywords.end()) return {identifier, TokenType::KEYWORD};
    if (types.find(identifier) != types.end()) return {identifier, TokenType::TYPE}; 
    return {identifier, TokenType::ID}; 
}

TokenType Lexer::map_operator_to_token_type(const std::string& op) {
    static const std::unordered_map<std::string, TokenType> operator_map = {
        {"+", TokenType::PLUS}, {"-", TokenType::MINUS}, {"*", TokenType::MULTIPLY}, {"/", TokenType::DIVIDE},
        {"%", TokenType::MODULO}, {"++", TokenType::INCREMENT}, {"--", TokenType::DECREMENT},
        {"==", TokenType::EQUAL}, {"!=", TokenType::NOT_EQUAL}, {">", TokenType::GREATER}, {"<", TokenType::LESS},
        {">=", TokenType::GREATER_EQUAL}, {"<=", TokenType::LESS_EQUAL}, {"&&", TokenType::AND}, {"||", TokenType::OR},
        {"=", TokenType::ASSIGN}, {"+=", TokenType::PLUS_ASSIGN}, {"-=", TokenType::MINUS_ASSIGN},
        {"*=", TokenType::MULTIPLY_ASSIGN}, {"/=", TokenType::DIVIDE_ASSIGN}, {"%=", TokenType::MODULO_ASSIGN},
        {"&", TokenType::BIT_AND}, {"|", TokenType::BIT_OR}, {"^", TokenType::BIT_XOR}, {"~", TokenType::BIT_NOT},
        {"<<", TokenType::LEFT_SHIFT}, {">>", TokenType::RIGHT_SHIFT}
    };

    auto it = operator_map.find(op);
    if (it != operator_map.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown operator token type for: " + op);
}

void print_tokens(const std::vector<Token>& tokens) {
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        std::cout << i << " token " << tokens[i].value << " --> type " << static_cast<int>(tokens[i].type) << std::endl;
    }
}
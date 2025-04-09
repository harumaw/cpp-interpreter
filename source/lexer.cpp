#include "lexer.hpp"
#include "token.hpp"
#include <iostream>
#include <fstream>
#include <cctype>
#include <unordered_map>
#include <set>


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

            if (std::isdigit(current) || current == '"' || current == '\'' || 
                (current == '-' && offset + 1 < line.size() && std::isdigit(line[offset + 1]))) {
                tokens.push_back(extract_literal(line));
            } else if (std::isalpha(current) || current == '_') {
                tokens.push_back(extract_id(line));
            } else if (operator_char.find(current) != std::string::npos) {
                tokens.push_back(extract_operator(line));
            } else if (std::ispunct(current)) {
                tokens.push_back(extract_punctuator(line));
            } else {
                offset++;
            }
        }
    }
    tokens.push_back({TokenType::END, ""});
    return tokens;
}

Token Lexer::extract_literal(const std::string& line) {
    if (std::isdigit(line[offset])) {
        std::size_t size = 0;

        while (std::isdigit(line[offset + size])) ++size;

        if (line[offset + size] == '.') {
            ++size;
            while (std::isdigit(line[offset + size])) ++size;
        }

        std::string value = line.substr(offset, size);
        offset += size;
        return {TokenType::LITERAL_NUM, value};
    }

    if (line[offset] == '\'') {
        if (offset + 2 < line.size() && line[offset + 2] == '\'') {
            std::string value = line.substr(offset, 3);
            offset += 3;
            return {TokenType::LITERAL_CHAR, value};
        }
    }

    if (line[offset] == '"') {
        std::size_t start = offset + 1;
        std::size_t end = line.find('"', start);
        if (end != std::string::npos) {
            std::string value = line.substr(offset, end - offset + 1);
            offset = end + 1;
            return {TokenType::LITERAL_STRING, value};
        }
    }

    return {TokenType::END, ""};
}

Token Lexer::extract_id(const std::string& line) {
    std::size_t start = offset;
    while (offset < line.size() && (std::isalnum(line[offset]) || line[offset] == '_')) {
        offset++;
    }

    std::string value = line.substr(start, offset - start);

 
    if (types.find(value) != types.end()) {
        return {TokenType::TYPE, value};
    }

   
    if (keywords.find(value) != keywords.end()) {
        return {keywords[value], value};
    }

    return {TokenType::ID, value};
}

Token Lexer::extract_operator(const std::string& line) {
    if (offset + 1 < line.size()) {
        std::string two_char_op = line.substr(offset, 2);
        auto it = operators.find(two_char_op);
        if (it != operators.end()) {
            offset += 2;  
            return {it->second, two_char_op};  
        }
    }

    
    std::string one_char_op(1, line[offset]);
    auto it = operators.find(one_char_op);
    if (it != operators.end()) {
        offset++;  
        return {it->second, one_char_op};  
    }

    offset++;  
    return {TokenType::END, ""};  
}

Token Lexer::extract_punctuator(const std::string& line) {
    for (const auto& punct : punctuators) {
        if (line.compare(offset, punct.first.size(), punct.first) == 0) {
            offset += punct.first.size();
            return {punct.second, punct.first};
        }
    }

    offset++;
    return {TokenType::END, ""};
}

Token Lexer::extract_keyword(const std::string& line) {
    std::size_t start = offset;
    while (offset < line.size() && (std::isalnum(line[offset]) || line[offset] == '_')) {
        offset++;
    }

    std::string value = line.substr(start, offset - start);
    if (keywords.find(value) != keywords.end()) {
        return {keywords[value], value};
    }

    return {TokenType::ID, value};
}

Token Lexer::extract_type(const std::string& line) {
    std::size_t start = offset;
    while (offset < line.size() && (std::isalnum(line[offset]) || line[offset] == '_')) {
        offset++;
    }

    std::string value = line.substr(start, offset - start);
    if (types.find(value) != types.end()) {
        return {TokenType::TYPE, value};
    }

    return {TokenType::ID, value};
}





std::string Lexer::token_type_to_string(TokenType type) {
    switch (type) {
    
        case TokenType::LITERAL_NUM: return "LITERAL_NUM";
        case TokenType::LITERAL_CHAR: return "LITERAL_CHAR";
        case TokenType::LITERAL_STRING: return "LITERAL_STRING";
        case TokenType::TYPE: return "TYPE";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::MODULO: return "MODULO";
        case TokenType::POWER: return "POWER";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS_ASSIGN: return "PLUS_ASSIGN";
        case TokenType::MINUS_ASSIGN: return "MINUS_ASSIGN";
        case TokenType::MULTIPLY_ASSIGN: return "MULTIPLY_ASSIGN";
        case TokenType::DIVIDE_ASSIGN: return "DIVIDE_ASSIGN";
        case TokenType::MODULO_ASSIGN: return "MODULO_ASSIGN";
        case TokenType::RIGHT_SHIFT_ASSIGN: return "RIGHT_SHIFT_ASSIGN";
        case TokenType::LEFT_SHIFT_ASSIGN: return "LEFT_SHIFT_ASSIGN";
        case TokenType::AND_ASSIGN: return "AND_ASSIGN";
        case TokenType::XOR_ASSIGN: return "XOR_ASSIGN";
        case TokenType::OR_ASSIGN: return "OR_ASSIGN";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::LESS: return "LESS";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::NOT: return "NOT";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::QUESTION: return "QUESTION";
        case TokenType::BIT_AND: return "BIT_AND";
        case TokenType::BIT_OR: return "BIT_OR";
        case TokenType::BIT_XOR: return "BIT_XOR";
        case TokenType::BIT_NOT: return "BIT_NOT";
        case TokenType::LEFT_SHIFT: return "LEFT_SHIFT";
        case TokenType::RIGHT_SHIFT: return "RIGHT_SHIFT";
        case TokenType::INCREMENT: return "INCREMENT";
        case TokenType::DECREMENT: return "DECREMENT";
        case TokenType::INDEX_LEFT: return "INDEX_LEFT";
        case TokenType::INDEX_RIGHT: return "INDEX_RIGHT";
        case TokenType::DOT: return "DOT";
        case TokenType::ARROW: return "ARROW";
        case TokenType::COMMA: return "COMMA";
        case TokenType::COLON: return "COLON";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::PARENTHESIS_LEFT: return "PARENTHESIS_LEFT";
        case TokenType::PARENTHESIS_RIGHT: return "PARENTHESIS_RIGHT";
        case TokenType::BRACE_LEFT: return "BRACE_LEFT";
        case TokenType::BRACE_RIGHT: return "BRACE_RIGHT";
        case TokenType::PUNCTUATOR: return "PUNCTUATOR";
        case TokenType::ID: return "ID";
        case TokenType::KEYWORD: return "KEYWORD";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::FOR: return "FOR";
        case TokenType::WHILE: return "WHILE";
        case TokenType::STRUCT: return "STRUCT";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::CONST: return "CONST";
        case TokenType::DO: return "DO";
        case TokenType::FALSE: return "FALSE";
        case TokenType::TRUE: return "TRUE";
        case TokenType::RETURN: return "RETURN";
        case TokenType::END: return "END";

        default: return "UNKNOWN";
    }
}

void Lexer::print_tokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << token.value << " ---> " << token_type_to_string(token.type) << std::endl;
    }
}

std::set<std::string> Lexer::types = {"int", "float", "double", "char", "bool", "size_t", "void"};
std::unordered_map<std::string, TokenType> Lexer::operators = {
    {"+", TokenType::PLUS},
    {"-", TokenType::MINUS},
    {"*", TokenType::MULTIPLY},
    {"/", TokenType::DIVIDE},
    {"%", TokenType::MODULO},
    {"^^", TokenType::POWER},
    {"=", TokenType::ASSIGN},
    {"+=", TokenType::PLUS_ASSIGN},
    {"-=", TokenType::MINUS_ASSIGN},
    {"*=", TokenType::MULTIPLY_ASSIGN},
    {"/=", TokenType::DIVIDE_ASSIGN},
    {"%=", TokenType::MODULO_ASSIGN},
    {">>=", TokenType::RIGHT_SHIFT_ASSIGN},
    {"<<=", TokenType::LEFT_SHIFT_ASSIGN},
    {"&=", TokenType::AND_ASSIGN},
    {"^=", TokenType::XOR_ASSIGN},
    {"|=", TokenType::OR_ASSIGN},
    {"==", TokenType::EQUAL},
    {"!=", TokenType::NOT_EQUAL},
    {">", TokenType::GREATER},
    {"<", TokenType::LESS},
    {">=", TokenType::GREATER_EQUAL},
    {"<=", TokenType::LESS_EQUAL},
    {"!", TokenType::NOT},
    {"&&", TokenType::AND},
    {"||", TokenType::OR},
    {"?", TokenType::QUESTION},
    {"&", TokenType::BIT_AND},
    {"|", TokenType::BIT_OR},
    {"^", TokenType::BIT_XOR},
    {"~", TokenType::BIT_NOT},
    {"<<", TokenType::LEFT_SHIFT},
    {">>", TokenType::RIGHT_SHIFT},
    {"++", TokenType::INCREMENT},
    {"--", TokenType::DECREMENT},
    {".", TokenType::DOT},
    {"->", TokenType::ARROW}
};
std::unordered_map<std::string, TokenType> Lexer::punctuators = {
    {",", TokenType::COMMA},
    {".", TokenType::DOT},
    {":", TokenType::COLON},
    {";", TokenType::SEMICOLON},
    {"{", TokenType::BRACE_LEFT},
    {"}", TokenType::BRACE_RIGHT},
    {"(", TokenType::PARENTHESIS_LEFT},
    {")", TokenType::PARENTHESIS_RIGHT},
    {"[", TokenType::INDEX_LEFT},
    {"]", TokenType::INDEX_RIGHT}
};
std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"for", TokenType::FOR},
    {"while", TokenType::WHILE},
    {"struct", TokenType::STRUCT},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"const", TokenType::CONST},
    {"do", TokenType::DO},
    {"false", TokenType::FALSE},
    {"true", TokenType::TRUE},
    {"return", TokenType::RETURN}
};



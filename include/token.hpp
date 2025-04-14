#pragma once
#include <string>
#include <iostream>

enum class TokenType {
     
        LITERAL_NUM,
        LITERAL_CHAR,
        LITERAL_STRING,
    
     
        TYPE,
    
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
        MODULO,
        POWER,
    
    
        ASSIGN,
        PLUS_ASSIGN,
        MINUS_ASSIGN,
        MULTIPLY_ASSIGN,
        DIVIDE_ASSIGN,
        MODULO_ASSIGN,
        RIGHT_SHIFT_ASSIGN,
        LEFT_SHIFT_ASSIGN,
        AND_ASSIGN,
        XOR_ASSIGN,
        OR_ASSIGN,
    
        EQUAL,
        NOT_EQUAL,
        GREATER,
        LESS,
        GREATER_EQUAL,
        LESS_EQUAL,
    
    
        NOT,
        AND,
        OR,
        QUESTION,
 
        BIT_AND,
        BIT_OR,
        BIT_XOR,
        BIT_NOT,
        LEFT_SHIFT,
        RIGHT_SHIFT,
    

        INCREMENT,
        DECREMENT,
    

        INDEX_LEFT,
        INDEX_RIGHT,
        DOT,
        ARROW,
    

        COMMA,
        COLON,
        SEMICOLON,
        PARENTHESIS_LEFT,
        PARENTHESIS_RIGHT,
        BRACE_LEFT,
        BRACE_RIGHT,
        PUNCTUATOR,
    
     
        ID,
        KEYWORD,
        IF,
        ELSE,
        FOR,
        WHILE,
        STRUCT,
        SIZEOF,
        STATICASSERT,
        BREAK,
        CONTINUE,
        CONST,
        DO,
        FALSE,
        TRUE,
        RETURN,
    
      
        END
};

struct Token {
    TokenType type;
    std::string value;

    bool operator== (const TokenType type) {
        return this->type == type;
    }

    bool operator!= (const TokenType type) {
        return !(*this == type);
    }

    Token(const Token& other) : type(other.type), value(other.value) {}

    Token(Token&& other) noexcept : type(other.type), value(std::move(other.value)) {}

    Token(TokenType type) : type(type) {}

    Token(TokenType type, std::string value) : type(type), value(value) {}
};
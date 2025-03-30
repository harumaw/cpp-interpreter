#pragma once

#include <string>

enum class TokenType {
    // Литералы
    LITERAL, 

    // Типы данных
    TYPE, 

    // Арифметические операторы
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO, POWER, 

    // Присваивание
    ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, MULTIPLY_ASSIGN, DIVIDE_ASSIGN, MODULO_ASSIGN, 
    RIGHT_SHIFT_ASSIGN, LEFT_SHIFT_ASSIGN, AND_ASSIGN, XOR_ASSIGN, OR_ASSIGN, 

    // Сравнение
    EQUAL, NOT_EQUAL, GREATER, LESS, GREATER_EQUAL, LESS_EQUAL, 

    // Логические операторы
    NOT, AND, OR, QUESTION, 

    // Побитовые операторы
    BIT_AND, BIT_OR, BIT_XOR, BIT_NOT, LEFT_SHIFT, RIGHT_SHIFT, 

    // Инкремент и декремент
    INCREMENT, DECREMENT, 

    // Индексация
    INDEX_LEFT, INDEX_RIGHT, 

    // Другие символы
    DOT, ARROW, COMMA, SEMICOLON, PARENTHESIS_LEFT, PARENTHESIS_RIGHT, 
    BRACE_LEFT, BRACE_RIGHT, 

    // Прочие токены
    PUNCTUATOR, ID, KEYWORD, END
};

struct Token {
    std::string value;
    TokenType type; 

    // Оператор сравнения с типом токена
    bool operator==(TokenType other_type) const {
        return type == other_type;
    }

    // Оператор сравнения со строковым значением
    bool operator==(const std::string& other_value) const {
        return value == other_value;
    }
};
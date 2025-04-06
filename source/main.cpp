#include <iostream>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "printer.hpp"




int main() {
    try {
        // Создаем лексер и токенизируем входной файл
        Lexer lexer("example.txt");
        std::vector<Token> tokens = lexer.tokenize();

        // Выводим токены для отладки
        Lexer::print_tokens(tokens);

        // Создаем парсер и парсим токены
        Parser parser(tokens);
        auto translation_unit = parser.parse();

        // Используем Printer для вывода результата парсинга
        Printer printer;
        printer.visit(*translation_unit);
    } catch (const std::exception& e) {
        std::cerr << "Ошибка парсинга: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
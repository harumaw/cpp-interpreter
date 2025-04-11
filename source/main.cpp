#include <iostream>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "printer.hpp"




int main() {
    try {
        Lexer lexer("example.txt");
        std::vector<Token> tokens = lexer.tokenize();


    //Lexer::print_tokens(tokens);

        Parser parser(tokens);
        auto translation_unit = parser.parse();

        Printer printer;
        printer.visit(*translation_unit);
    } catch (const std::exception& e) {
        std::cerr << "Ошибка парсинга:" << e.what() << std::endl;
        return 1;
    }

    return 0;
}
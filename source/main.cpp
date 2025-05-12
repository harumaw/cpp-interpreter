#include <iostream>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "analyzer.hpp"   
#include "printer.hpp"

int main() {
    try {

        Lexer lexer("example.txt");
        auto tokens = lexer.tokenize();
        Lexer::print_tokens(tokens);
        std::cout << "lexer end" << std::endl;

        Parser parser(tokens);
        auto translation_unit = parser.parse();
        
        std::cout << "parser end" << std::endl;

        Analyzer analyzer;
        analyzer.analyze(*translation_unit);

        std::cout << "analyzer end" << std::endl;


        Printer printer;
        printer.visit(*translation_unit);

    } catch (const std::runtime_error& e) {
        std::cerr << "Semantic error: " << e.what() << std::endl;
        return 2;
  
    return 0;
}
}
#include <iostream>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "analyzer.hpp"
#include "printer.hpp"
#include "executer.hpp"  

int main() {
    try {
        Lexer  lexer("example.txt");
        auto   tokens = lexer.tokenize();
        //Lexer::print_tokens(tokens);
        std::cout << "lexer end\n";

        Parser parser(tokens);
        auto   translation_unit = parser.parse();
        std::cout << "parser end\n";

        
        Analyzer analyzer;
        analyzer.analyze(*translation_unit);
        std::cout << "analyzer end\n";

        const auto& errors = analyzer.getErrors();
        if (!errors.empty()) {
            std::cerr << "Semantic errors found (" << errors.size() << "):\n";
            for (auto& msg : errors) {
                std::cerr << "  --> " << msg << "\n";
            }
            return 2;  
        }

        
        Execute executor;
        executor.symbolTable = analyzer.getScope();
        executor.execute(*translation_unit);
        
        std::cout << "executer end\n";

       Printer printer;
        printer.visit(*translation_unit);
        return 0;
        

        
 

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

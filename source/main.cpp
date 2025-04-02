#include <iostream>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"




int main() {
    Lexer lexer("example.txt");
    std::vector<Token> tokens = lexer.tokenize();
        
    Lexer::print_tokens(tokens);
        
        
    /*Parser parser(tokens);
    std::vector<node> parsed_nodes = parser.parse();
    for (const auto& n : parsed_nodes) {
        if (!n) {
            std::cout << "Null node detected!" << std::endl;
            continue;
        }
        std::cout << "Node type: " << typeid(*n).name() << std::endl;
        
    }
    */
    
    
}
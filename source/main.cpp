#include <iostream>
#include <lexer.hpp>




int main() {
    Lexer lexer("example.txt");
	std::vector<Token> tokens = lexer.tokenize();
	
	print_tokens(tokens);
	
	return 0;
}
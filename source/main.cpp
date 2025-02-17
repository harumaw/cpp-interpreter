#include <iostream>
#include <include/lexer.h>




int main() {
    Lexer lexer("example.txt");
	std::vector<Token> tokens = lexer.tokenize();
	
	print_tokens(tokens);
	

}
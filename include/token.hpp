#pragma once

#include <string>

enum class TokenType{
    NUMBER, IDENTIFIER, OPERATOR, KEYWORD, PUNCTUATOR, TYPE, END

    // number - chislo
    // identifier - nazvanie peremennih
    // operator - 
    /* +  -  *  /  %  
    =  ==  !=  <  >  <=  >=  
    &&  ||  !  
    ++  --  */

    // keyword - if, else, for, while, return, break, continue

    // punctuator - (), {}, [], ;, ,, .

    // type - int, float, char, string, bool, double, void
    // end - eof
};

struct Token {
    std::string value;
    TokenType type; 

    bool operator==(TokenType other_type) const{
        return type == other_type;
    }

    bool operator==(const std::string& other_value) const {
		return value == other_value;
	}
};
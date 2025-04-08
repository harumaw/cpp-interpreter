#include <string>
#include <vector>
#include <memory> 

#include "parser.hpp"

#include <string>
#include <vector>
#include <memory> 

#include "parser.hpp"

Parser::Parser(
    const std::vector<Token>& tokens
    ) : tokens(tokens), offset(0) {}


    std::shared_ptr<RootNode> Parser::parse() {
        std::vector<std::shared_ptr<ASTNode>> ast_nodes;
    
        while (!match_token(TokenType::END)) {
            if (check_token(TokenType::TYPE)) {
                // Если это тип, значит, это декларация
                ast_nodes.push_back(parse_declaration());
            } else {
                // В противном случае это оператор
                ast_nodes.push_back(parse_statement());
            }
        }
    
        return std::make_shared<RootNode>(ast_nodes);
    }



declaration Parser::parse_declaration() {
    if (match_pattern(TokenType::TYPE, TokenType::ID, TokenType::PARENTHESIS_LEFT) || match_pattern(TokenType::TYPE, TokenType::MULTIPLY, TokenType::ID, TokenType::PARENTHESIS_LEFT)) {
        //std::cout << "jump";
        return parse_function_declaration();
    } else if (match_pattern(TokenType::TYPE, TokenType::ID) || match_pattern(TokenType::TYPE, TokenType::MULTIPLY, TokenType::ID)) {
        return parse_var_declaration();
    } else {
        //std::cout << "stop moment";
        throw std::runtime_error("Declaration : Unexpected token " + tokens[offset].value);
    }
}

func_declaration Parser::parse_function_declaration() {
    auto type = extract_token(TokenType::TYPE);
    auto declarator = parse_declarator();
    

    extract_token(TokenType::PARENTHESIS_LEFT);
    

    std::vector<std::shared_ptr<ParameterDeclaration>> args;
    if (!match_token(TokenType::PARENTHESIS_RIGHT)) {
        while (true) {
            args.push_back(parse_parameter_declaration());
            

            if (tokens[offset].value == ",") {
                ++offset;
            } else if (tokens[offset].value == ")") {
                ++offset;
                break;
            } else {
                throw std::runtime_error("Missing closing parenthesis");
            }
        }
    }


    std::shared_ptr<CompoundStatement> body;
    if (match_token(TokenType::BRACE_LEFT)) {
        body = parse_compound_statement();
    } else if (!match_token(TokenType::SEMICOLON)) {
        throw std::runtime_error("Unexpected token");
    }
    return std::make_shared<FuncDeclaration>(type, declarator, args, body);
}

parameter_declaration Parser::parse_parameter_declaration() {
    // dobavitb proverku na otsutstvie parametrov
    auto type = extract_token(TokenType::TYPE);
    auto declarator = parse_init_declarator();


    return std::make_shared<ParameterDeclaration>(type, declarator);
}
var_declaration Parser::parse_var_declaration() {
    auto type = extract_token(TokenType::TYPE);
    std::vector<std::shared_ptr<Declaration::InitDeclarator>> declarator_list;

    while (true) {
        declarator_list.push_back(parse_init_declarator());
        if (match_token(TokenType::COMMA)) {
            continue;
        } else if (match_token(TokenType::SEMICOLON)) {
            break;
        } else {
            throw std::runtime_error("Var : Unexpected token " + tokens[offset].value);
        }
    }

    return std::make_shared<VarDeclaration>(type, declarator_list);
}

init_declarator Parser::parse_init_declarator() {

    auto declarator = parse_declarator();
    std::shared_ptr<Expression> initializer;

    if (match_token(TokenType::ASSIGN)) {
        initializer = parse_expression();
    }

    return std::make_shared<Declaration::InitDeclarator>(declarator, initializer);
}

declarator Parser::parse_declarator() {
    if (match_pattern(TokenType::MULTIPLY, TokenType::ID)) {
        extract_token(TokenType::MULTIPLY);
        return std::make_shared<Declaration::PtrDeclarator>(extract_token(TokenType::ID));
    } else if (check_token(TokenType::ID)) {
        return std::make_shared<Declaration::NoPtrDeclarator>(extract_token(TokenType::ID));
    } else {
        throw std::runtime_error("Declarator : Unexpected token " + tokens[offset].value);
    }
}
statement Parser::parse_statement() {
    if (match_token(TokenType::BRACE_LEFT)) {
        return parse_compound_statement();
    } else if (match_token(TokenType::IF)) {
        return parse_conditional_statement();
    } else if (check_token(TokenType::WHILE) || check_token(TokenType::FOR)) {
        return parse_loop_statement();
    } else if (check_token(TokenType::RETURN, TokenType::BREAK, TokenType::CONTINUE)) {
        return parse_jump_statement();
    } else if (check_token(TokenType::TYPE)) {
        return parse_declaration_statement();
    } else {
        return parse_expression_statement();
    }
}
compound_statement Parser::parse_compound_statement() {
    std::vector<std::shared_ptr<Statement>> statements;
    while (!match_token(TokenType::BRACE_RIGHT)) {
        statements.push_back(parse_statement());
    }
    return std::make_shared<CompoundStatement>(statements);
}

conditional_statement Parser::parse_conditional_statement() {
    extract_token(TokenType::PARENTHESIS_LEFT); 
    auto if_condition = parse_expression();    
    extract_token(TokenType::PARENTHESIS_RIGHT); 
    auto if_statement = parse_statement();    
    auto if_branch = std::make_pair(if_condition, if_statement);

    std::shared_ptr<Statement> else_branch;
    if (match_token(TokenType::ELSE)) {      
        else_branch = parse_statement();
    }

    return std::make_shared<ConditionalStatement>(if_branch, else_branch);
}

loop_statement Parser::parse_loop_statement() {
    if (match_token(TokenType::WHILE)) {
        return parse_while_statement();
    } else if (match_token(TokenType::FOR)) {
        return parse_for_statement();
    } else {
        throw std::runtime_error("Unexpected token in loop statement:" + tokens[offset].value);
    }
}

while_statement Parser::parse_while_statement() {
    extract_token(TokenType::PARENTHESIS_LEFT);
    auto condition = parse_expression();
    extract_token(TokenType::PARENTHESIS_RIGHT);
    auto statement = parse_statement();
    return std::make_shared<WhileStatement>(condition, statement);
}

for_statement Parser::parse_for_statement() {


    extract_token(TokenType::PARENTHESIS_LEFT); 
  

    std::shared_ptr<ASTNode> initialization;
    if (!check_token(TokenType::SEMICOLON)) {  
        if (check_token(TokenType::TYPE)) {  
            initialization = parse_var_declaration(); 
        } else {
            initialization = parse_expression_statement();  
        }
    } else {
        extract_token(TokenType::SEMICOLON);  
    }
   

    std::shared_ptr<Expression> condition;
    if (!check_token(TokenType::SEMICOLON)) {  
        condition = parse_expression();  
    }
    extract_token(TokenType::SEMICOLON);  
 

    std::shared_ptr<Expression> increment;
    if (!check_token(TokenType::PARENTHESIS_RIGHT)) {  
        increment = parse_expression();  
    }
    extract_token(TokenType::PARENTHESIS_RIGHT); 

    extract_token(TokenType::BRACE_LEFT);
    auto body = parse_statement();  

    extract_token(TokenType::BRACE_RIGHT);

    return std::make_shared<ForStatement>(initialization, condition, increment, body);
}





jump_statement Parser::parse_jump_statement() {
    if (match_token(TokenType::BREAK)) {
        return parse_break_statement();
    } else if (match_token(TokenType::CONTINUE)) {
        return parse_continue_statement();
    } else {
        extract_token(TokenType::RETURN);
        return parse_return_statement();
    }
}

break_statement Parser::parse_break_statement() {
    extract_token(TokenType::SEMICOLON);
    return std::make_shared<BreakStatement>();
}

continue_statement Parser::parse_continue_statement() {
    extract_token(TokenType::SEMICOLON);
    return std::make_shared<ContinueStatement>();
}

return_statement Parser::parse_return_statement() {
    std::shared_ptr<Expression> expression = nullptr;
    if (!check_token(TokenType::SEMICOLON)) {
        expression = parse_expression();
    }

    extract_token(TokenType::SEMICOLON);

    return std::make_shared<ReturnStatement>(expression); 
}

declaration_statement Parser::parse_declaration_statement() {
    auto declaration = parse_var_declaration();
    return std::make_shared<DeclarationStatement>(declaration);
}

expression_statement Parser::parse_expression_statement() {
    auto expression = parse_expression();
    extract_token(TokenType::SEMICOLON);
    return std::make_shared<ExpressionStatement>(expression);
}


std::shared_ptr<Expression> Parser::parse_expression() {
    return parse_binary_expression(0);
}

binary_expression Parser::parse_binary_expression(int min_precedence) {
    binary_expression lhs = parse_unary_expression();
    for (auto op = tokens[offset].value; operator_precedences.contains(op) && operator_precedences.at(op) >= min_precedence; op = tokens[offset].value) {
        ++offset;
        lhs = std::make_shared<BinaryOperation>(op, lhs, parse_binary_expression(operator_precedences.at(op)));
    }
    return lhs;
}

unary_expression Parser::parse_unary_expression() {
    if (auto op = tokens[offset].value; unary_operators.contains(op)) {
        ++offset;
        return std::make_shared<PrefixExpression>(op, parse_unary_expression());
    }
    return parse_postfix_expression();
}

postfix_expression Parser::parse_postfix_expression() {
    std::shared_ptr<PostfixExpression> base = parse_primary_expression();

    while (true) {
        if (match_token(TokenType::INCREMENT)) {
            base = std::make_shared<PostfixIncrementExpression>(base);
        } else if (match_token(TokenType::DECREMENT)) {
            base = std::make_shared<PostfixDecrementExpression>(base);
        } else if (match_token(TokenType::PARENTHESIS_LEFT)) {
            std::vector<std::shared_ptr<Expression>> args = parse_function_call_expression();
            base = std::make_shared<FunctionCallExpression>(base, args);
        } else if (match_token(TokenType::INDEX_LEFT)) {
            std::shared_ptr<Expression> index = parse_subscript_expression();
            base = std::make_shared<SubscriptExpression>(base, index);
        } else {
            break; 
        }
    }

    return base;
}

func_param Parser::parse_function_call_expression() {
    std::vector<std::shared_ptr<Expression>> args;

    if (!match_token(TokenType::PARENTHESIS_RIGHT)) {
        do {
            args.push_back(parse_expression());  
        } while (match_token(TokenType::COMMA));  
    }

    extract_token(TokenType::PARENTHESIS_RIGHT);  

    return args; 
}

std::shared_ptr<Expression> Parser::parse_subscript_expression() {
    auto index = parse_expression();
    extract_token(TokenType::INDEX_RIGHT);
    return index;
}

primary_expression Parser::parse_primary_expression() {
    if (check_token(TokenType::LITERAL_NUM)) {
        return std::make_shared<IntLiteral>(extract_token(TokenType::LITERAL_NUM));
    } else if (check_token(TokenType::LITERAL_CHAR)) {
        return std::make_shared<CharLiteral>(extract_token(TokenType::LITERAL_CHAR));
    } else if (check_token(TokenType::LITERAL_STRING)) {
        return std::make_shared<StringLiteral>(extract_token(TokenType::LITERAL_STRING));
    } else if (check_token(TokenType::TRUE, TokenType::FALSE)) {
        return std::make_shared<BoolLiteral>(extract_token(TokenType::TRUE, TokenType::FALSE));
    } else if (check_token(TokenType::ID)) {
        return std::make_shared<IdentifierExpression>(extract_token(TokenType::ID));
    } else if (match_token(TokenType::PARENTHESIS_LEFT)) {
        return parse_parenthesized_expression();
    } else {
        throw std::runtime_error("Unexpected token " + tokens[offset].value);
    }
}

parentsized_expression Parser::parse_parenthesized_expression() {
    auto expression = parse_expression();
    extract_token(TokenType::PARENTHESIS_RIGHT);
    return std::make_shared<ParenthesizedExpression>(expression);
}///////////////////////////////////////////////////////////////

template<typename... Args>
bool Parser::check_token(const Args&... expected) {
	return ((tokens[offset].type == expected) || ...);
}

template<typename... Args>
bool Parser::match_token(const Args&... expected) {
	bool match_found = check_token(expected...);
	if (match_found) {
		++offset;
	}
	return match_found;
}

template<typename... Args>
std::string Parser::extract_token(const Args&... expected) {
	if (!((tokens[offset].type == expected) || ...)) {
		throw std::runtime_error("Extract token : Unexpected token " + tokens[offset].value);
	}
	return tokens[offset++].value;
}

template<typename... Args>
bool Parser::match_pattern(const Args&... expected) {
	std::size_t i = offset;
	return ((tokens[i++].type == expected) && ...);
}


const std::unordered_map<std::string, int> Parser::operator_precedences = {
	{"=", 0}, {"+=", 0}, {"-=", 0}, {"*=", 0}, {"/=", 0}, {"%=", 0}, {"**=", 0},
	{"||", 1},
	{"&&", 2},
	{"==", 3}, {"!=", 3},
	{"<", 4}, {"<=", 4}, {">", 4}, {">=", 4},
	{"+", 5}, {"-", 5},
	{"*", 6}, {"/", 6}, {"%", 6},
	{"^", 7}
};

const std::unordered_set<std::string> Parser::unary_operators = {
	"+", "-", "&", "*", "!", "++", "--"
};
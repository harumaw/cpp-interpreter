#include <string>
#include <vector>
#include <memory> 

#include "parser.hpp"

Parser::Parser(
    const std::vector<Token>& tokens
    ) : tokens(tokens), offset(0) {}


std::shared_ptr<TranslationUnit> Parser::parse() {
    std::vector<std::shared_ptr<ASTNode>> ast_nodes;
    
    while (!match_token(TokenType::END)) {
        if (is_type_specifier()) {
            ast_nodes.push_back(parse_declaration());
        } else {
            ast_nodes.push_back(parse_statement());
        }
    }
    
    return std::make_shared<TranslationUnit>(ast_nodes);
}
 

bool Parser::is_type_specifier() {
     if (check_token(TokenType::CONST))     return true;
    if (check_token(TokenType::NAMESPACE)) return true;
    if (check_token(TokenType::TYPE)      ||
        check_token(TokenType::STRUCT))   return true;
    if (check_token(TokenType::ID) &&
        peek_token(1).type == TokenType::ID)
        return true;
    return false;
}

declaration Parser::parse_declaration() {
    if(match_token(TokenType::NAMESPACE)){
        return parse_namespace_declaration();
    }
    else if (match_pattern(TokenType::TYPE, TokenType::ID, TokenType::PARENTHESIS_LEFT) || match_pattern(TokenType::TYPE, TokenType::MULTIPLY, TokenType::ID, TokenType::PARENTHESIS_LEFT)) {
        return parse_function_declaration();
    }
    else if(match_pattern(TokenType::TYPE, TokenType::ID, TokenType::INDEX_LEFT)){
        return parse_array_declaration();

    }
    else if (match_pattern(TokenType::TYPE, TokenType::ID) || match_pattern(TokenType::TYPE, TokenType::MULTIPLY, TokenType::ID) || match_pattern(TokenType::ID, TokenType::ID) || match_pattern(TokenType::CONST, TokenType::TYPE, TokenType::ID)) {
        return parse_var_declaration();
    }
    else if (match_pattern(TokenType::STRUCT, TokenType::ID)) {
        return parse_struct_declaration();  
    }
    else {
            throw std::runtime_error("Declaration : Unexpected token " + tokens[offset].value);
    }
}

name_space_declaration Parser::parse_namespace_declaration() {

    auto name = extract_token(TokenType::ID);
    extract_token(TokenType::BRACE_LEFT);

    std::vector<std::shared_ptr<Declaration>> decls;

    while (!match_token(TokenType::BRACE_RIGHT)) {
        decls.push_back(parse_declaration());
    }

    return std::make_shared<NameSpaceDeclaration>(name, decls);
}


struct_declaration Parser::parse_struct_declaration() {
    extract_token(TokenType::STRUCT);

    std::string struct_name = tokens[offset].value;
    extract_token(TokenType::ID);
    

    std::vector<std::shared_ptr<VarDeclaration>> members;

 
    extract_token(TokenType::BRACE_LEFT);  
    
    while (!check_token(TokenType::BRACE_RIGHT)) {
        auto field = parse_var_declaration();
        members.push_back(field); // ispravit
    }
    
    extract_token(TokenType::BRACE_RIGHT);  
    extract_token(TokenType::SEMICOLON); // bespolezen
 
    return std::make_shared<StructDeclaration>(struct_name, members);
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


array_declaration Parser::parse_array_declaration() {
    auto type = extract_token(TokenType::TYPE);
    auto name = extract_token(TokenType::ID);

    extract_token(TokenType::INDEX_LEFT);

   
    std::shared_ptr<Expression> size = nullptr;
    if (!match_token(TokenType::INDEX_RIGHT)) {
        size = parse_expression();
        extract_token(TokenType::INDEX_RIGHT);
    }

    
    std::vector<std::shared_ptr<Expression>> init_list;
    if (match_token(TokenType::ASSIGN)) {
        extract_token(TokenType::BRACE_LEFT);

        if (!match_token(TokenType::BRACE_RIGHT)) {
            do {
                init_list.push_back(parse_expression());
            } while (match_token(TokenType::COMMA));
            extract_token(TokenType::BRACE_RIGHT);
        }
    }

    extract_token(TokenType::SEMICOLON);
    return std::make_shared<ArrayDeclaration>(type, name, size, init_list);
}

parameter_declaration Parser::parse_parameter_declaration() {
    auto type = extract_token(TokenType::TYPE);
    auto declarator = parse_init_declarator();


    return std::make_shared<ParameterDeclaration>(type, declarator);
}


var_declaration Parser::parse_var_declaration() { //maybe rework
    bool is_const = false;


    std::cout << "test";
    if(match_token(TokenType::CONST)){
        is_const = true;
    }

    
    std::string type;
    if (check_token(TokenType::TYPE)) {
        type = extract_token(TokenType::TYPE);
    } else if (check_token(TokenType::ID)) {
        type = extract_token(TokenType::ID);
        
    } else {
        throw std::runtime_error("Var : expected type or identifier, but got " + tokens[offset].value);
    }

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

    return std::make_shared<VarDeclaration>(is_const, type, declarator_list);
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
        return std::make_shared<Declaration::SimpleDeclarator>(extract_token(TokenType::ID));
    } else {
        throw std::runtime_error("Declarator : Unexpected token " + tokens[offset].value);
    }
}


//block nazvatb
statement Parser::parse_statement() {
    if (match_token(TokenType::BRACE_LEFT)) {
        return parse_compound_statement();
    } else if (match_token(TokenType::IF)) {
        return parse_conditional_statement();
    } else if (check_token(TokenType::WHILE) || check_token(TokenType::FOR) || check_token(TokenType::DO)) {
        return parse_loop_statement();
    } else if (check_token(TokenType::RETURN, TokenType::BREAK, TokenType::CONTINUE)) {
        return parse_jump_statement();
    } else if(match_token(TokenType::STATICASSERT)){
        return parse_staticassert_statement();
    } else if (is_type_specifier()) {
        return parse_declaration_statement();
    } else {
        return parse_expression_statement();
    }
}
compound_statement Parser::parse_compound_statement() { 
    std::vector<std::shared_ptr<Statement>> statements;
    while (!match_token(TokenType::BRACE_RIGHT)) {
        statements.push_back(parse_statement());
        if(offset >= tokens.size()){
            throw std::runtime_error("missing }");
        }
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

loop_statement Parser::parse_loop_statement() { //do-while
    if (match_token(TokenType::WHILE)) {
        return parse_while_statement();
    } else if (match_token(TokenType::FOR)) {
        return parse_for_statement();
    } else if (match_token(TokenType::DO)){
        return parse_do_while_statement();
    } 
    else {

        throw std::runtime_error("Unexpected token in loop statement:" + tokens[offset].value);
    }
}

do_while_statement Parser::parse_do_while_statement(){
    auto statement = parse_statement();
    extract_token(TokenType::WHILE);
    extract_token(TokenType::PARENTHESIS_LEFT);
    auto condition = parse_expression();
    extract_token(TokenType::PARENTHESIS_RIGHT);
    extract_token(TokenType::SEMICOLON);
    return std::make_shared<DoWhileStatement>(statement, condition);
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
            initialization = parse_declaration(); 
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

   
    auto body = parse_statement();  


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
    std::shared_ptr<Expression> expression;
    if (!check_token(TokenType::SEMICOLON)) {
        expression = parse_expression();
    }

    extract_token(TokenType::SEMICOLON);

    return std::make_shared<ReturnStatement>(expression); 
}

declaration_statement Parser::parse_declaration_statement() {
    auto declaration = parse_declaration();
    return std::make_shared<DeclarationStatement>(declaration);
}

expression_statement Parser::parse_expression_statement() { // mozhet bit nullptr
    auto expression = parse_expression();
    extract_token(TokenType::SEMICOLON);
    return std::make_shared<ExpressionStatement>(expression);
}


stat_assert Parser::parse_staticassert_statement(){
    extract_token(TokenType::PARENTHESIS_LEFT);

    auto condition = parse_expression();

    extract_token(TokenType::COMMA);

    auto msg = extract_token(TokenType::LITERAL_STRING);
    extract_token(TokenType::PARENTHESIS_RIGHT);
    extract_token(TokenType::SEMICOLON);

    return std::make_shared<StaticAssertStatement>(condition, msg);
}
//comma dobavit, logical
expression Parser::parse_expression(){
    return parse_assignment();
}

expression Parser::parse_comma_expression(){
    auto left = parse_assignment();
    while(match_token(TokenType::COMMA)){
        auto op = tokens[offset - 1].value;
        auto right = parse_assignment();
        left = std::make_shared<BinaryOperation>(op, left, right);
    } // while dlya levoy if rigt
    return left;
}


expression Parser::parse_assignment(){ 
    auto left = parse_ternary_expression(); //logical or
    if (match_token(TokenType::ASSIGN, TokenType::PLUS_ASSIGN, TokenType::MINUS_ASSIGN, TokenType::MULTIPLY_ASSIGN, TokenType::DIVIDE_ASSIGN, TokenType::MODULO_ASSIGN)) {
        auto op = tokens[offset - 1].value;
        auto right = parse_assignment();
        left = std::make_shared<BinaryOperation>(op, left, right);
    }
    return left;
}

expression Parser::parse_ternary_expression(){ // int x = 5; int y = (x > 0) ? 10 : 20;
    auto condition = parse_logical_or_expression();
    while (match_token(TokenType::QUESTION)) {
        auto true_expr = parse_expression();
        extract_token(TokenType::COLON);
        auto false_expr = parse_expression();
        condition = std::make_shared<TernaryExpression>(condition, true_expr, false_expr);
    } 
    return condition;
}

expression Parser::parse_logical_or_expression(){
    auto left = parse_logical_and_expression();
    if(match_token(TokenType::OR)){
        auto op = tokens[offset -1].value;
        auto right = parse_logical_or_expression();
        left = std::make_shared<BinaryOperation>(op, left, right);
    }
    return left;
}

expression Parser::parse_logical_and_expression(){
    auto left = parse_equality_expression();
    if(match_token(TokenType::AND)){
        auto op = tokens[offset -1].value;
        auto right = parse_logical_and_expression();
        left = std::make_shared<BinaryOperation>(op, left, right);
    }
    return left;
}

expression Parser::parse_equality_expression(){
    auto left = parse_compared_expression();
    while(match_token(TokenType::EQUAL, TokenType::NOT_EQUAL)){
        auto op = tokens[offset -1].value;
        auto right = parse_compared_expression();
        left = std::make_shared<BinaryOperation>(op, left, right);
    }
    return left;
    
}



expression Parser::parse_compared_expression(){ // 4 < 20 < 2    EQUAL,
    auto left = parse_sum_expression();
    if(match_token(TokenType:: EQUAL, TokenType:: NOT_EQUAL, TokenType:: GREATER, TokenType:: LESS, TokenType:: GREATER_EQUAL, TokenType:: LESS_EQUAL)){
        auto op = tokens[offset -1].value;
        auto right = parse_compared_expression();
        left = std::make_shared <BinaryOperation>(op, left, right);
    }
    return left;
}


expression Parser::parse_sum_expression(){ // 1+3+4
    auto left = parse_mul_expression();
    while(match_token(TokenType::PLUS, TokenType::MINUS)){
        auto op = tokens[offset-1].value;
        auto right = parse_mul_expression();
        left = std::make_shared<BinaryOperation>(op, left, right);
    }
    return left;
}


expression Parser::parse_mul_expression(){
    auto left = parse_pow_expression();
    while(match_token(TokenType::MULTIPLY, TokenType::DIVIDE)){
        auto op = tokens[offset-1].value;
        auto right = parse_pow_expression();
        left = std::make_shared<BinaryOperation>(op, left, right);
    }
    return left;
}

expression Parser::parse_pow_expression(){
    auto left = parse_unary_expression();
    if(match_token(TokenType::POWER)){
        auto op = tokens[offset -1].value;
        auto right = parse_pow_expression();
        left = std::make_shared<BinaryOperation>(op, left, right);
    }
    return left;
}

expression Parser::parse_unary_expression(){ // a 2 ls + - and logical
    if(match_token(TokenType::INCREMENT, TokenType:: DECREMENT)){
        auto op = tokens[offset-1].value;
        auto base = parse_postfix_expression();
        return std::make_shared<PrefixExpression>(op, base);
    }
    return parse_postfix_expression();
}

expression Parser::parse_postfix_expression() {

    if (match_token(TokenType::SIZEOF)) {
        if (match_token(TokenType::PARENTHESIS_LEFT)) {
            if (check_token(TokenType::TYPE)) {
                auto type_name = extract_token(TokenType::TYPE);
                extract_token(TokenType::PARENTHESIS_RIGHT);
                return std::make_shared<SizeOfExpression>(type_name);
            } else {
                auto expr = parse_expression();
                extract_token(TokenType::PARENTHESIS_RIGHT);
                return std::make_shared<SizeOfExpression>(expr);
            }
        } else {
            throw std::runtime_error("Expected '(' after sizeof");
        }
    }

    //std::cout << tokens[offset].value << std::endl;
    auto left = parse_base();

    while (check_token(TokenType::INCREMENT, TokenType::DECREMENT, TokenType::INDEX_LEFT, 
                       TokenType::PARENTHESIS_LEFT, TokenType::DOT, TokenType::SCOPE)) {
        
        if (match_token(TokenType::INCREMENT)) {
            left = std::make_shared<PostfixIncrementExpression>(left);
        }
        if (match_token(TokenType::DECREMENT)) {
            left = std::make_shared<PostfixDecrementExpression>(left);
        }
        if (match_token(TokenType::INDEX_LEFT)) {
            auto arg = parse_expression();
            extract_token(TokenType::INDEX_RIGHT);
            left = std::make_shared<SubscriptExpression>(left, arg);
        }
        if (match_token(TokenType::PARENTHESIS_LEFT)) {
            std::cout << tokens[offset].value << std::endl;
            std::vector<std::shared_ptr<Expression>> args;
            while (!check_token(TokenType::PARENTHESIS_RIGHT)) {
                auto arg = parse_expression();
                args.push_back(arg);
                std::cout << tokens[offset].value << std::endl;
                if (match_token(TokenType::COMMA)) {
                    continue; 
                } else if (check_token(TokenType::PARENTHESIS_RIGHT)) {
                    break; 
                } else {
                    throw std::runtime_error("Expected comma or closing parenthesis");
                }
            }
            extract_token(TokenType::PARENTHESIS_RIGHT); 
            left = std::make_shared<FunctionCallExpression>(left, args);
        }
        if (match_token(TokenType::DOT)) {
            auto member = extract_token(TokenType::ID);
            left = std::make_shared<StructMemberAccessExpression>(left, member);
        }
        if(match_token(TokenType::SCOPE)){
            auto member = extract_token(TokenType::ID);
            left = std::make_shared<NameSpaceAcceptExpression>(left, member);
        }
    }

    return left;
}



expression Parser::parse_base() {
    if (match_token(TokenType::PARENTHESIS_LEFT)) {
        auto inner = parse_expression();
        extract_token(TokenType::PARENTHESIS_RIGHT);
        return inner;
    }

    if (check_token(TokenType::LITERAL_CHAR)) {
        return std::make_shared<CharLiteral>(extract_token(TokenType::LITERAL_CHAR));
    }
    if (check_token(TokenType::LITERAL_NUM)) {
        auto value = extract_token(TokenType::LITERAL_NUM);
        if (value.find('.') != std::string::npos)
            return std::make_shared<FloatLiteral>(value);
        else
            return std::make_shared<IntLiteral>(value);
    }
    if (check_token(TokenType::LITERAL_STRING)) {
        return std::make_shared<StringLiteral>(extract_token(TokenType::LITERAL_STRING));
    }
    if (check_token(TokenType::TRUE, TokenType::FALSE)) {
        auto val = tokens[offset++].value;
        return std::make_shared<BoolLiteral>(val);
    }

    if (check_token(TokenType::ID)) {
        auto name = extract_token(TokenType::ID);
        return std::make_shared<IdentifierExpression>(name);
    }



    throw std::runtime_error("parse base error " + tokens[offset].value);
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
bool Parser::check_token(const Args&... expected) {
	return ((tokens[offset].type == expected) || ...);
}

template<typename... Args>
bool Parser::match_pattern(const Args&... expected) {
	std::size_t i = offset;
	return ((tokens[i++].type == expected) && ...);
} 

template<typename... Args>
std::string Parser::extract_token(const Args&... expected) {
	if (!((tokens[offset].type == expected) || ...)) {
		throw std::runtime_error("Extract token : Unexpected token " + tokens[offset].value);
	}
	return tokens[offset++].value;
}

Token Parser::peek_token(int lookahead) {
    if (offset + lookahead < tokens.size()) {
        return tokens[offset + lookahead];
    } else {
        return Token{TokenType::END, ""}; 
    }
}


const std::unordered_set<std::string> Parser::unary_operators = {
	"+", "-", "&", "*", "!", "++", "--"
};
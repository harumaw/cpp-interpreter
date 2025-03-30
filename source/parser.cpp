#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_set>
#include "parser.hpp"

#define MIN_PRECEDENCE 10

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), offset(0) {}


std::vector<node> Parser::parse() {
	std::vector<node> nodes;
	//std::cout << "Tokens vector size: " << tokens.size() << std::endl;
	
	while (offset < tokens.size() && tokens[offset].type != TokenType::END) {
		nodes.push_back(parse_base());
	}
	
	//std::cout << "parsing is ended\n";
	
	for (const auto& node : nodes) {
		if (node) {
			node->print();
		}
	} 
	
	return nodes;
};




decl Parser::parse_variable_declaration() {
    std::string var_type = getTypeFrom_string(tokens[offset++].value);
    decl_lst vars_lst;
    
   // std::cout << "Var Dec parsing\n" << std::endl;
    
    while (tokens[offset].value != ";" && tokens[offset].value != ")") {
        if ((tokens[offset].value == "," || tokens[offset].value == "',") && tokens[offset + 1].type != TokenType::TYPE) {
            ++offset; 
            continue;
        } else if (tokens[offset].value == "," && tokens[offset + 1].type == TokenType::TYPE) {
        	++offset;
        	return std::make_shared<VariableDeclarationNode>(var_type, vars_lst);
        }

        if (tokens[offset].type == TokenType::IDENTIFIER) {
            std::string& var = tokens[offset++].value;

            if (tokens[offset].value == "=") {
            	if (var_type == "char") {
            		if (tokens[++offset].value == "'") {
            			if (tokens[++offset].value != "'") {
            				std::string sym = tokens[offset++].value;
            				expr sym_node = std::make_shared<CharacterNode>(sym[0]);
            				vars_lst.push_back(std::make_pair(var, sym_node));
            				
            				if (tokens[offset].value == "'" || tokens[offset].value == "',") {
            					++offset;
            				} else if (tokens[offset].value == "';") {
            					++offset;
            					return std::make_shared<VariableDeclarationNode>(var_type, vars_lst);
            				} else {
            					throw std::runtime_error("Syntax error - missing ' after char value");
            				}
            			} else {
            				throw std::runtime_error("Syntax error - missing ' before char value");
            			}
            			
            		} 
            	
            	} else if (var_type == "string") {
            		if (tokens[++offset].value == "\"") {
            			if (tokens[offset + 1].value != "\"") {
            				vars_lst.push_back(std::make_pair(var, parse_string()));
            				if (tokens[offset].value == "\"" || tokens[offset].value == "\",") {
            					++offset;
            				} else if (tokens[offset].value == "\";") {
            					++offset;
            					return std::make_shared<VariableDeclarationNode>(var_type, vars_lst);
            				} else {
            					throw std::runtime_error("Syntax error - missing \" after char value");
            				}
            			} else {
            				throw std::runtime_error("Syntax error - missing \" before char value");
            			}
            		} else if (match_type(TokenType::IDENTIFIER) && tokens[offset + 1].type == TokenType::OPERATOR) {
            			expr str_expr = binary_parse();
            			vars_lst.push_back(std::make_pair(var, str_expr));
            			return std::make_shared<VariableDeclarationNode>(var_type, vars_lst);
            		}
            	} else {
            		if (((tokens[offset + 1].type == TokenType::IDENTIFIER || tokens[offset + 1].type == TokenType::LITERAL || tokens[offset + 1].type == TokenType::OPERATOR) && (tokens[offset + 2].type == TokenType::OPERATOR || (tokens[offset + 2].type == TokenType::IDENTIFIER && tokens[offset + 3].value != "("))) || tokens[offset + 1].value == "(") {
		    		++offset;
		    		expr binary_var_val = binary_parse();
		    	
		    		vars_lst.push_back(std::make_pair(var, binary_var_val));

		    		if (match_type(TokenType::END)) {
		    			return std::make_shared<VariableDeclarationNode>(var_type, vars_lst);
		    		} else if (tokens[offset + 1].value == ";") {
		    			++offset;
		    		} else if (tokens[offset - 1].value == ";" && tokens[offset + 1].type != TokenType::END) {
		    			--offset;  		
		    		} else {
		    			return std::make_shared<VariableDeclarationNode>(var_type, vars_lst);
		    		}	
            		} else if (tokens[offset + 1].type == TokenType::IDENTIFIER && (tokens[offset + 2].value == "(" || tokens[offset + 2].value == "();")) {
		    		++offset;
		    		expr func_value = parse_function_call();
		    		vars_lst.push_back(std::make_pair(var, func_value));
		    		
		    		if (match_value(");")) {
		    			++offset;
		    			return std::make_shared<VariableDeclarationNode>(var_type, vars_lst);
		    		} else if (match_value("),")) {
		    			++offset;
		    		} else if (tokens[offset - 1].type == TokenType::PUNCTUATOR) {
		    			return std::make_shared<VariableDeclarationNode>(var_type, vars_lst);
		    		}
            		
            		} else if (tokens[offset + 1].type == TokenType::OPERATOR && (tokens[offset + 2].type == TokenType::IDENTIFIER || tokens[offset + 2].type == TokenType::LITERAL)) {
            			++offset;
            			expr var_value;
            			if (tokens[offset + 1].type == TokenType::IDENTIFIER) {
            				var_value = parse_unary();
            			} else {
            				std::string num_value = tokens[offset].value + tokens[offset + 1].value;
            				var_value = std::make_shared<NumberNode>(std::stod(num_value));
            				offset += 2;
            			}
            			
            			vars_lst.push_back(std::make_pair(var, var_value));	
            			if (match_value(";")) {
					++offset;
					return std::make_shared<VariableDeclarationNode>(var_type, vars_lst); 
				}
            		
            		} else {
		    		++offset;
				expr var_value = std::static_pointer_cast<Expression>(parse_token());
				vars_lst.push_back(std::make_pair(var, var_value));
				
			
				//std::cout << offset << std::endl;
				if (match_value(";")) {
					++offset;
					return std::make_shared<VariableDeclarationNode>(var_type, vars_lst); 
				} else if (match_type(TokenType::IDENTIFIER) || !match_type(TokenType::PUNCTUATOR)) {
					throw std::runtime_error("Syntax error: unexpected token after variable initialization");
				}
			}
		}
            
            } else if (tokens[offset].value == ";" || tokens[offset].value == ",") {
            	vars_lst.push_back(std::make_pair(var, nullptr));
            
            } else if (tokens[offset].value == ")" || tokens[offset].value == ");") {
            	vars_lst.push_back(std::make_pair(var, nullptr));

            	return std::make_shared<VariableDeclarationNode>(var_type, vars_lst);            	
            } else {
            	//std::cout << offset << std::endl;
                throw std::runtime_error("Syntax error - after variable name initialization expected value or ';' or ',', but there's not");
            }
            
        } else {
		//std::cout << offset << ": Token: " << std::endl;         
		throw std::runtime_error("Syntax error - unexpected token instead of variable");
        }
    }
    
    
    if (tokens[offset].value != ";") {
    	throw std::runtime_error("Syntax error - expected ';' at the end of variable initialization");
    }
      
    ++offset;
   
    return std::make_shared<VariableDeclarationNode>(var_type, vars_lst);	
}


decl Parser::parse_function_declaration() {
	std::string function_type = getTypeFrom_string(tokens[offset++].value);
	std::string function_name = tokens[offset++].value;
	std::vector<decl> args;
		
	if (tokens[offset].value == "()") {
		++offset;
	} else {
		++offset;
	
		while (tokens[offset].value != ")" && tokens[offset].value != ");") {
			if (tokens[offset].type == TokenType::TYPE && tokens[offset + 1].type == TokenType::IDENTIFIER) {
				args.push_back(parse_variable_declaration());
			} else {
				throw std::runtime_error("Syntax error - expected variable declaration inside function arguments");
			}
		}
	}
	
	if (match_value(")") || match_value(");")) {
		++offset;
	}
	
	
	st block_st;
	
	if (tokens[offset].value == "{") {
		if (tokens[offset + 1].value != "}") { 
			block_st = parse_block_statement();
			if (match_value("}")) ++offset;
		} else {
			block_st = nullptr;
			offset += 2;
		}
	} else if (tokens[offset].value == ";" || tokens[offset].value == "{}") {
		block_st = nullptr;
		++offset;
	}
	
	
		
	return std::make_shared<FunctionDeclarationNode>(function_type, function_name, args, block_st);
}





st Parser::parse_if_statement() {
	expr cond_expr;
	st block_st;
	
	if (match_value("if")) {
		++offset;
		
		if (match_value("(") && tokens[offset + 1].value != ")") {
			++offset;
		} else if (match_value("()")) {
			throw std::runtime_error("Syntax error - empty condition");
		} else {
			throw std::runtime_error("Syntax error - missing '(' before condition");
		}
		
		if (match_type(TokenType::IDENTIFIER) || match_type(TokenType::LITERAL)) {
			if (tokens[offset + 1].type == TokenType::OPERATOR && ((tokens[offset + 2].type == TokenType::IDENTIFIER || tokens[offset + 2].type == TokenType::LITERAL) || tokens[offset + 2].type == TokenType::OPERATOR)) {
				cond_expr = binary_parse();
				++offset;
				if (match_value(")")) {
					++offset;
				}
			} else if (tokens[offset + 1].type == TokenType::OPERATOR && tokens[offset + 2].type == TokenType::PUNCTUATOR) {
				cond_expr = parse_unary();
				++offset;
			} else {
				cond_expr = std::make_shared<IdentifierNode>(tokens[offset++].value);
			}
		} else if (match_type(TokenType::OPERATOR)) {
			if (tokens[offset + 1].type == TokenType::IDENTIFIER && tokens[offset + 2].type == TokenType::OPERATOR) {
				cond_expr = binary_parse();
			} else {
				cond_expr = parse_unary();
			}
		}
		
		if (match_value("{") && tokens[offset + 1].value != "}") {
			block_st = parse_block_statement();
			if (match_value("}") && tokens[offset - 1].value != "}") {
				++offset;
			}
		} else if (match_value("{}")) {
			block_st = nullptr;
			++offset;
		} else if (match_value("{") && tokens[offset + 1].value == "}") {
			block_st = nullptr;
			offset += 2;
		} else {
			throw std::runtime_error("Syntax error - missing block statement");
		}
		
		return std::make_shared<IF_Statement_Node>(cond_expr, block_st);
	} else if (match_type(TokenType::END)) {
		return nullptr;
	}
	
	std::cout << offset << std::endl;
	throw std::runtime_error("Unexpected token");
}


st Parser::parse_else_statement() {
	st block_st;
		
	if (match_value("else")) {
		++offset;
		if (match_value("{") && tokens[offset + 1].value != "}") {
			block_st = parse_block_statement();
			if (match_value("}") && tokens[offset - 1].value != "}") {
				++offset;
			}
		} else if (match_value("{}")) {
			block_st = nullptr;
			++offset;
		} else if (match_value("{") && tokens[offset + 1].value == "}") {
			block_st = nullptr;
			offset += 2;
		} else {
			throw std::runtime_error("Syntax error - missing block statement");
		}
		
		std::cout << offset << "else statement parsed\n";
		return std::make_shared<ELSE_Statement_Node>(block_st);
	} else {
		return nullptr;
	}
	
	std::cout << offset << std::endl;
	throw std::runtime_error("Unexpected token");
}


st Parser::parse_conditional() {	
	st if_st, else_st;
	
	if_st = parse_if_statement();
	else_st = parse_else_statement();
	
	return std::make_shared<ConditionalNode>(if_st, else_st);
}


st Parser::parse_while() {
	expr cond_expr;
	st block_st;
	
	if (match_value("(") && tokens[offset + 1].value != ")") {
		++offset;
	} else if (match_value("()")) {
		throw std::runtime_error("Syntax error - empty condition");
	} else {
		throw std::runtime_error("Syntax error - missing '(' before condition");
	}
	
	if (match_type(TokenType::IDENTIFIER) || match_type(TokenType::LITERAL)) {
		if (tokens[offset + 1].type == TokenType::OPERATOR && ((tokens[offset + 2].type == TokenType::IDENTIFIER || tokens[offset + 2].type == TokenType::LITERAL) || tokens[offset + 2].type == TokenType::OPERATOR)) {
			cond_expr = binary_parse();
			++offset;
			if (match_value(")")) {
				++offset;
			}
		} else if (tokens[offset + 1].type == TokenType::OPERATOR && tokens[offset + 2].type == TokenType::PUNCTUATOR) {
			cond_expr = parse_unary();
			if (match_value(")")) {
				++offset;
			}
		} else {
			cond_expr = std::make_shared<IdentifierNode>(tokens[offset++].value);
		}
	} else if (match_type(TokenType::OPERATOR)) {
		if (tokens[offset + 1].type == TokenType::IDENTIFIER && tokens[offset + 2].type == TokenType::OPERATOR) {
			cond_expr = binary_parse();
		} else {
			cond_expr = parse_unary();
		}
	}
	
	if (match_value("{") && tokens[offset + 1].value != "}") {
		block_st = parse_block_statement();
	} else if (match_value("{}")) {
		block_st = nullptr;
		++offset;
	} else if (match_value("{") && tokens[offset + 1].value == "}") {
		block_st = nullptr;
		offset += 2;
	} else {
		throw std::runtime_error("Syntax error - missing block statement");
	}
	
	return std::make_shared<WhileLoopNode>(cond_expr, block_st);
}


st Parser::parse_for() {
	std::vector<decl> var_decls; 
	expr cond_expr;
	std::vector<expr> iters;
	st block_st;
	
	if (match_value("(")) {
		++offset;
	} else {
		//std::cout << offset << std::endl;
		throw std::runtime_error("Syntax error: missing '(' before loop 'for' expression");
	}
	
	if (match_type(TokenType::TYPE)) {
		var_decls.push_back(parse_variable_declaration());	
	}
	
	cond_expr = binary_parse();
	
	std::cout << "Loop cond parsed\n";
	
	while (!match_value(")") && !match_value("{")) {
		iters.push_back(binary_parse());
		//std::cout << offset << std::endl;

		if (match_value(",") && tokens[offset + 1].value != ")") {
			++offset;
		}
	}
	
	if (match_value(")")) {
		++offset;
	}
	
	std::cout << "iters parsed" << offset << std::endl; 
	
	if (match_value("{") && tokens[offset + 1].value != "}") {
		block_st = parse_block_statement();
		if (match_value("}")) {
			++offset;
		}
	} else if (match_value("{}")) {
		block_st = nullptr;
		++offset;
	} else if (match_value("{") && tokens[offset + 1].value == "}") {
		block_st = nullptr;
		offset += 2;
	} else {
		throw std::runtime_error("Syntax error - missing block statement");
	}
	
	std::cout << "For loop parsed\n" << offset;
	
	return std::make_shared<ForLoopNode>(var_decls, cond_expr, iters, block_st);
}


st Parser::parse_loop() {
	std::string keyword = tokens[offset++].value;
	
	if (keyword == "while") {
		return parse_while();
	} else if (keyword == "for") {
		return parse_for();
	} 
	
	throw std::runtime_error("Parsing Error - unreachable scope");
}


st Parser::parse_block_statement() {
	std::vector<node> block_nodes;	
	++offset;
	
	while (tokens[offset].value != "}") {
		if (tokens[offset].type == TokenType::TYPE && tokens[offset + 1].type == TokenType::IDENTIFIER && tokens[offset + 2].value != "(") {
			block_nodes.push_back(parse_variable_declaration());
		} else if (tokens[offset].type == TokenType::IDENTIFIER && (tokens[offset + 1].value == "(" || tokens[offset + 1].value == "(\"")) {
			block_nodes.push_back(parse_function_call());
		} else if (match_type(TokenType::END)) {
			std::cout << "\nproblem: " << offset << std::endl;
			throw std::runtime_error("Syntax error - missing '}' at the end of condition block statement");
		} else {
			block_nodes.push_back(parse_token());
		}
	}
	
	if (tokens[offset].value != "}") {
		std::cout << "\nproblem: " << offset << std::endl;
		throw std::runtime_error("Syntax error - missing '}' at the end of condition block statement");
	}
	
	++offset;
	return std::make_shared<BlockStatementNode>(block_nodes);
}


st Parser::parse_jump() {
	if (match_value("return")) {
		++offset;
		if (!match_value(";")) {
			expr value = std::static_pointer_cast<Expression>(parse_token());
			
			if (match_type(TokenType::LITERAL)) {
				++offset;
			}
			
			if (match_value(";")) {
				++offset;
			} else if (tokens[offset - 1].value == ";") {
			
			} else {
				throw std::runtime_error("Syntax error - missing ';' at the end of jump word");
			}
			
			return std::make_shared<ReturnNode>(value);
		} else {
			++offset;
			return std::make_shared<ReturnNode>(nullptr);
		}
		
	} else if (match_value("break")) {
		++offset;
		if (!match_value(";")) {
			throw std::runtime_error("Syntax error - missing ';' at the end of jump word");
		} else {
			++offset;
			return std::make_shared<BreakNode>();
		}
	} else if (match_value("continue")) {
		++offset;
		if (!match_value(";")) {
			throw std::runtime_error("Syntax error - missing ';' at the end of jump word");
		} else {
			++offset;
			return std::make_shared<ContinueNode>();
		}
	}
	
	return nullptr;
}




expr Parser::binary_parse() {
	return parse_binary_expression(MIN_PRECEDENCE);
}

expr Parser::parse_binary_expression(int min_precedence) {
	if (match_type(TokenType::LITERAL) && (tokens[offset + 1].value == "=" || tokens[offset + 1].value == "+=" || tokens[offset + 1].value == "-=" || tokens[offset + 1].value == "*=" || tokens[offset + 1].value == "/=")) {
		throw std::runtime_error("Syntax error: left operand is rvalue: can't be assigned a value in constant value");
	}
	
	expr lhs;
	if (match_type(TokenType::IDENTIFIER)) { 
		if (tokens[offset + 1].type == TokenType::OPERATOR && (tokens[offset + 2].type == TokenType::OPERATOR || tokens[offset + 2].type == TokenType::PUNCTUATOR)) {
			lhs = parse_unary();
		} else if ((tokens[offset + 1].type == TokenType::OPERATOR || tokens[offset + 1].type == TokenType::PUNCTUATOR) && (tokens[offset + 2].type == TokenType::IDENTIFIER || tokens[offset + 2].type == TokenType::LITERAL)) {
			std::string id = tokens[offset++].value;
			lhs = std::make_shared<IdentifierNode>(id);
		} else {
			lhs = std::static_pointer_cast<Expression>(parse_token());	
		}
	} else if (match_type(TokenType::OPERATOR)) {
		if (tokens[offset + 1].type == TokenType::IDENTIFIER && (tokens[offset + 2].type == TokenType::OPERATOR && tokens[offset + 2].type == TokenType::IDENTIFIER)) {
			lhs = parse_unary();
		} else {
			lhs = std::static_pointer_cast<Expression>(parse_token());	
		}
	} else {
		lhs = std::static_pointer_cast<Expression>(parse_token());
	}

	for (auto op = tokens[offset].value; num_operators.contains(op) && num_operators.at(op) <= min_precedence; op = tokens[offset].value) {
		++offset;
		auto rhs = parse_binary_expression(num_operators.at(op));
		lhs = std::make_shared<BinaryNode>(op, lhs, rhs);
	}
	
	if (match_value(";")) {
		++offset;
	}
	
	//std::cout << offset << std::endl;
	
	return lhs;
}




expr Parser::parse_prefix() {
	std::string op;
	expr branch;
	
	op = tokens[offset++].value;	 
	std::string id = tokens[offset++].value; 
	branch = std::make_shared<IdentifierNode>(id);
	
	std::cout << offset << "- problem:" << std::endl; 
	
	if (tokens[offset].value == ";" || tokens[offset].value == ");" || tokens[offset].value == ")" || tokens[offset].value == ",") {
		++offset;
	} else {
		throw std::runtime_error("Syntax error - missing ';' at the end of unary prefix expression");
	}
      	
      	return std::make_shared<Prefix_UnaryNode>(op, branch);
}


expr Parser::parse_postfix() {
	std::string op;
	expr branch;
	
	std::string id = tokens[offset++].value;
    	branch = std::make_shared<IdentifierNode>(id);

	if (match_type(TokenType::OPERATOR)) {
		op = tokens[offset++].value;
	} else {
		throw std::runtime_error("Syntax error - Unexpected token instead of unary operator");
	}
	
	std::cout << offset << "- problem:" << std::endl;
	
	if (tokens[offset].value == ";" || tokens[offset].value == ");" || tokens[offset].value == ")" || tokens[offset].value == ",") {
		++offset;
	} else {
		throw std::runtime_error("Syntax error - missing ';' at the end of unary postfix expression");
	}
	
	return std::make_shared<Postfix_UnaryNode>(op, branch);
}


expr Parser::parse_unary() {
	std::string op;
	expr branch;

	// PREFIX
	if (match_type(TokenType::OPERATOR)) {
		return parse_prefix();
	//POSTFIX
  	} else if (match_type(TokenType::IDENTIFIER)) {
	    	return parse_postfix();
      	} else {
      		throw std::runtime_error("Unexpected token");	
      	}
      	
      	return nullptr;
}


expr Parser::parse_function_call() {
	std::string name = tokens[offset++].value;
	std::vector<expr> args = parse_function_interior();

	return std::make_shared<FunctionNode>(name, args);
}


std::vector<expr> Parser::parse_function_interior() {
	std::vector<expr> args;
	if (match_value("();")) {
		++offset;
		return args;
	}
	
	if (match_value("(") || match_value("(\"") || match_value("((")) {
		if (match_value("(\"")) {
			args.push_back(parse_string());
			if (match_value("\");")) {
				++offset;
				return args;
			}
		}
			
		++offset;
	
		if (!match_value(");") && !match_value("\");")) {
			while (true) {
				if (match_type(TokenType::IDENTIFIER) && tokens[offset + 1].value == "(") {
					args.push_back(parse_function_call());
					if (tokens[offset].value == ")" || tokens[offset].value == "),") {
						++offset;
					} else if (tokens[offset].value == ");" || tokens[offset].value == "));") {
						++offset;
						break;
					}
				} else if (match_type(TokenType::IDENTIFIER) && (tokens[offset + 1] == "\");" || tokens[offset + 1] == "\"" || tokens[offset + 1] == "\",")) {
					args.push_back(std::make_shared<StringNode>(tokens[offset++].value));
					
					if (match_value("\");")) {
						++offset;
						break;
					}
				
				} else {
					args.push_back(binary_parse());
					if (tokens[offset].value == ",") {
						++offset;
					} else if (match_type(TokenType::END)) {
						break;
					} else {
						if (tokens[offset].value == ")" || tokens[offset].value == ")," || tokens[offset].value == ");") {
							++offset;
							break;
						} else if (tokens[offset].value == "));") {
							break;
						} else {
							if (tokens[offset - 1].type == TokenType::PUNCTUATOR) {
								break;
							}
						}	
					}
				}
			}
		}
	}
	
	return args;
}


expr Parser::parse_parenthesized_expression() {
	value_extract("(");
	auto node = binary_parse();
	if (match_value(")") || match_value(");") || match_value("));")) {
		++offset;
	} else {
		throw std::runtime_error("Syntax error - missing ')' at the end of parenthes");
	}
	
	return std::make_shared<ParenthesizedNode>(node);
}


expr Parser::parse_string() {
	if (match_value("(\"") || match_value("\"")) ++offset;
	
	std::string res;
	while (tokens[offset].value != "\");" && tokens[offset].value != "\"" && tokens[offset].value != "\"," && tokens[offset].value != "\";") {
		if (tokens[offset].type == TokenType::END) {
			throw std::runtime_error("Syntax error: invalid string perfomance");
		}
		
		res += tokens[offset].value;
		if (tokens[offset + 1].type != TokenType::PUNCTUATOR) res += " ";
		++offset;
	}
	
	return std::make_shared<StringNode>(res);
}




node Parser::parse_token() {
    if (match_type(TokenType::LITERAL)) {
        if (tokens[offset].value.front() == '"' && tokens[offset].value.back() == '"') {
            std::string str_value = tokens[offset++].value;
            str_value = str_value.substr(1, str_value.size() - 2);
            return std::make_shared<StringNode>(str_value);
        }
        else if (tokens[offset].value.front() == '\'' && tokens[offset].value.back() == '\'') {
            std::string char_value = tokens[offset++].value;
            if (char_value.size() == 3) { 
                return std::make_shared<CharacterNode>(char_value[1]);
            } else {
                throw std::runtime_error("Syntax error: invalid char literal");
            }
        }
        // Если это число
        else {
            return std::make_shared<NumberNode>(std::stod(type_extract(TokenType::LITERAL)));
        }
    } else if (match_type(TokenType::IDENTIFIER)) {
        if (tokens[offset + 1].type == TokenType::OPERATOR && tokens[offset + 2].type == TokenType::PUNCTUATOR) {
            return parse_unary();
        } else if (tokens[offset + 1].type == TokenType::OPERATOR &&
                  (tokens[offset + 2].type == TokenType::LITERAL || tokens[offset + 2].type == TokenType::IDENTIFIER)) {
            return binary_parse();
        } else { 
            if (auto identifier = type_extract(TokenType::IDENTIFIER); 
                tokens[offset].value == "(" || tokens[offset].value == "();") {
                return std::make_shared<FunctionNode>(identifier, parse_function_interior());
            } else {
                return std::make_shared<IdentifierNode>(identifier);
            }
        }
    } else if (match_type(TokenType::TYPE)) {
        if (tokens[offset + 1].type == TokenType::IDENTIFIER &&
            (tokens[offset + 2].value != "(" && tokens[offset + 2].value != "()")) {
            return parse_variable_declaration();
        } else if (tokens[offset + 1].type == TokenType::IDENTIFIER &&
                  (tokens[offset + 2].value == "(" || tokens[offset + 2].value == "()")) {
            return parse_function_declaration();
        }
    } else if (match_type(TokenType::KEYWORD) && keywords.contains(tokens[offset].value)) {
        if (tokens[offset].value == "if" || tokens[offset].value == "elif" || tokens[offset].value == "else") {
            return parse_conditional();
        } else if (tokens[offset].value == "while" || tokens[offset].value == "do" || tokens[offset].value == "for") {
            return parse_loop();
        } else if (tokens[offset].value == "return" || tokens[offset].value == "break" || tokens[offset].value == "continue") {
            return parse_jump();
        }
    } else if (match_type(TokenType::OPERATOR)) {
        return parse_unary();
    } else if (match_value("(")) {
        return parse_parenthesized_expression();
    } else if (match_value("{")) {
        return parse_block_statement();
    } else if (match_type(TokenType::END)) {
        return nullptr;
    } else {
        throw std::runtime_error(std::to_string(offset) + " Syntax error - unexpected token: " + tokens[offset].value);
    }
    throw std::runtime_error("Parsing Error - unreachable scope");
}


node Parser::parse_base() {
    if (is_global_scope()) {
        if (match_type(TokenType::TYPE)) {
            return parse_variable_declaration();
        } else if (match_type(TokenType::KEYWORD)) {  // 
            return parse_token();  
        } else {
            std::cout << "Ошибка на токене #" << offset << ": " << tokens[offset].value << std::endl;
            throw std::runtime_error("Syntax error - Unexpected token in global scope.");
        }
    } else {
        return parse_token();
    }
}


bool Parser::is_global_scope() const {
	if (!match_type(TokenType::TYPE)) {
		return true;
	} else if (match_value("{") || match_type(TokenType::KEYWORD)) { 
		return false;
	} else {
		return match_type(TokenType::TYPE) && tokens[offset + 1].type == TokenType::IDENTIFIER && (tokens[offset + 2].value != "(" && tokens[offset + 2].value != "()");
	} 
}

bool Parser::match_value(std::string value) const {
	return tokens[offset].value == value;
}

bool Parser::match_type(TokenType expected_type) const {
	return tokens[offset] == expected_type;
}

std::string Parser::value_extract(std::string value) {
	if (!match_value(value)) {
		//std::cout << offset << std::endl;
		throw std::runtime_error("Unexpected token value:" + tokens[offset].value);
	}
	return tokens[offset++].value;
}


std::string Parser::type_extract(TokenType expected_type) {
	if (!match_type(expected_type)) {
		//std::cout << offset << std::endl;
		throw std::runtime_error("Unexpected token type:" + tokens[offset].value);
	}
	return tokens[offset++].value;
}


std::string Parser::getTypeFrom_string(const std::string& type_string) {
	if (type_string == "int") {
		return "int";
	} else if (type_string == "float") {
		return "float";
	} else if (type_string == "char") {
		return "char";
	} else if (type_string == "double") {
		return "double";
	} else if (type_string == "string") {
		return "string";
	} else if (type_string == "bool") {
		return "bool";
	} else if (type_string == "void") {
		return "void";
	} else {
		return "";
	}
}


const std::unordered_map<std::string, int> Parser::num_operators = {
	{"^", 1}, {"*", 2},
	{"/", 2}, {"%", 2},
	{"+", 3}, {"-", 3},
	{"<", 4}, {">", 4},
	{"<=", 4}, {">=", 4},
	{"==", 5}, {"!=", 5},
	{"&", 6}, {"|", 7}, 
	{"&&", 8}, {"||", 9},
	{"=", 10}, {"+=", 10},
	{"-=", 10}, {"*=", 10},
	{"/=", 10}
}; 
const std::unordered_set<std::string> Parser::types = {
    "int", "float", "double", "char", "string", "bool", "void", "struct"
};

const std::unordered_set<std::string> Parser::keywords = {
    "if", "elif", "else", "while", "do", "for", "return", "break", "continue"
};

const std::unordered_set<std::string> Parser::operators = {
    "+", "-", "*", "/", "%", "^", "&", "|", "=", "==", "!=", ">=", "<=", ">", "<", "!", "&&", "||"
};

const std::unordered_set<std::string> unary_operators = {
    "+", "-", "++", "--", "*", "&", ".", "->"
};
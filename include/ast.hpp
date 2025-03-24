/*
ASTNode
├── Declaration - декларация
│   ├── VariableDeclarationNode +
│   ├── FunctionDeclarationNode +
│   ├── ArrayNode
├── Statement - операторы 
|   ├── BlockStatementNode +
│   ├── IF_Statement_Node +
│   ├── ELSE_Statement_Node +
│   ├── ConditionalNode +
│   ├── WhileLoopNode +
│   ├── ForLoopNode +
│   ├── ReturnStatementNode +
│
└── Expression - выражения 
    ├── BinaryNode +
    ├── Prefix_UnaryNode +
    ├── Postfix_UnaryNode +
    ├── IdentifierNode +
    ├── FunctionNode +
    ├── ParenthesizedNode +
    ├── NumberNode +
    ├── CharacterNode +
    ├── BoolNode +
    ├── StringNode +
    ├── ArrayAccessNode
    ├── StructNode
    ├── StructAccessNode
    ├── CastNode
    ├── SizeofNode
    ├── AssertNode Exit Node
    ├── TernaryNode

*/







#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <memory>    





enum class NodeType {
	VARIABLE_DECLARATION,
	FUNCTION_DECLARATION,
	BLOCK_STATEMENT,
	IF_STATEMENT,
	ELSE_STATEMENT,
	CONDITIONAL,
	WHILE_LOOP,
	FOR_LOOP,
	RETURN,
	BREAK,
	CONTINUE,
	BINARY,
	PREFIX_OP,
	POSTFIX_OP,
	FUNCTION,
	IDENTIFIER,
	NUMBER,
	STRING,
	BOOL,
	CHARACTER,
	PARENTHES,
	ARRAY,
	ARRAY_ACCESS
};




struct ASTNode {
	virtual ~ASTNode() = default;
	virtual NodeType type() const = 0;
	virtual void print() const = 0;
};

using node = std::shared_ptr<ASTNode>;





struct Declaration : public ASTNode {
	virtual ~Declaration() = default;
	virtual NodeType type() const override = 0;
	virtual void print() const override = 0;
};

using decl = std::shared_ptr<Declaration>;


struct Statement : public ASTNode {
	virtual ~Statement() = default;
	virtual NodeType type() const override = 0;
	virtual void print() const override = 0;
};

using st = std::shared_ptr<Statement>;


struct Expression : public ASTNode {
	virtual ~Expression() = default;
	virtual NodeType type() const override = 0;
	virtual void print() const override = 0;
};

using expr = std::shared_ptr<Expression>;



using decl_lst = std::vector<std::pair<std::string, expr>>;

struct VariableDeclarationNode : public Declaration {
	std::string var_type;
	decl_lst vars_list;
	
	VariableDeclarationNode(const std::string& type, const decl_lst vars_lst) : var_type(type), vars_list(vars_lst) {}
	
	NodeType type() const override { return NodeType::VARIABLE_DECLARATION;}
	
	void print() const override {
        std::cout << "Variable Declaration Node." << std::endl;
        std::cout << "Type: " << var_type << std::endl;
    
        for (const auto& var : vars_list) {
            std::cout << "Variable: " << var.first << " = ";
            
            if (var.second) {
                var.second->print(); 
            } else {
                std::cout << "null";
            }
    
            std::cout << "\n";
        }
    
        std::cout << "End of Variable Declaration Node\n" << std::endl;
    }
}; 

using var_decl = std::shared_ptr<VariableDeclarationNode>;

struct FunctionDeclarationNode : public Declaration {
	std::string function_type;
	std::string function_name;
	std::vector<decl> branches;
	st block_statement;
	
	FunctionDeclarationNode(const std::string& t, const std::string& name, const std::vector<decl>& branches, const st& block_st) : function_type(t), function_name(name), branches(branches), block_statement(block_st) {}
	
	NodeType type() const override { return NodeType::FUNCTION_DECLARATION; }
	
	void print() const override {
		std::cout << "Function Declaration Node.\n" << "___\n";
		std::cout << "Type: " << function_type << ", Function name: " << function_name << std::endl;
		
		for (const auto& arg_node : branches) {
			if (arg_node) {
				arg_node->print();
			}
		}
		
		std::cout << std::endl;
		if (block_statement) {
			block_statement->print();
		}
	}
};

using func_decl = std::shared_ptr<FunctionDeclarationNode>; 






struct IF_Statement_Node : public Statement {
	expr condition;
	st block_statement;
	
	IF_Statement_Node(const expr cond_expr, const st block_st) : condition(cond_expr), block_statement(block_st) {}
	
	NodeType type() const override { return NodeType::IF_STATEMENT; }
	
	void print() const override {
        std::cout << "If Statement Node:\n";
        if(condition) {
            std::cout << "Condition: ";
            condition->print();
        }
        if(block_statement) {
            std::cout << "If Body:\n";
            block_statement->print();
        }

        std::cout << "End of If Statement Node\n";
    }

};

using if_statement = std::shared_ptr<IF_Statement_Node>;


struct ELSE_Statement_Node : public Statement {
	st block_statement;
	
	ELSE_Statement_Node(const st block_st) : block_statement(block_st) {}
	
	NodeType type() const override { return NodeType::ELSE_STATEMENT; }
	
	void print() const override {
        std::cout << "Else Statement Node:\n";
        if(block_statement) {
            std::cout << "Else Body:\n";
            block_statement->print();
        }
        std::cout << "End of Else Statement Node\n";
    }
};

using else_statement = std::shared_ptr<ELSE_Statement_Node>;




struct ConditionalNode : public Statement {
	st if_statement;
	st else_statement;
	
	ConditionalNode(const st if_st, const st else_st) : if_statement(if_st), else_statement(else_st) {} 

	NodeType type() const override { return NodeType::CONDITIONAL; }
	
	void print() const override {
        std::cout << "Conditional Node:\n";
        if(if_statement) {
            std::cout << "If Statement:\n";
            if_statement->print();
        }
        if(else_statement) {
            std::cout << "Else Statement:\n";
            else_statement->print();
        }
        std::cout << "End of Conditional Node\n";
    
	} 
};

using cond = std::shared_ptr<ConditionalNode>;


struct LoopNode : public Statement {
	virtual ~LoopNode() = default;
	virtual NodeType type() const override = 0;
	virtual void print() const override = 0;
};

using loop = std::shared_ptr<LoopNode>;


struct WhileLoopNode : public LoopNode {
	expr condition_expr;
	st block_statement;
	
	WhileLoopNode(const expr cond_expr, const st block_st) : condition_expr(cond_expr), block_statement(block_st) {}
	
	NodeType type() const override { return NodeType::WHILE_LOOP; }
	
	void print() const override {
        std::cout << "While Loop Node:\n";
        if(condition_expr) {
            std::cout << "Condition: ";
            condition_expr->print();
        }
        if(block_statement) {
            std::cout << "Body:\n";
            block_statement->print();
        }
        std::cout << "End of While Loop Node\n";
    }
};

using while_loop = std::shared_ptr<WhileLoopNode>;


struct ForLoopNode : public LoopNode {
	std::vector<decl> var_decls;
	expr condition_expr;
	std::vector<expr> iterat;
	st block_statement;
	
	ForLoopNode(const std::vector<decl> vars, const expr cond, const std::vector<expr> iters, const st block_st) : var_decls(vars), condition_expr(cond), iterat(iters), block_statement(block_st) {}
	
	NodeType type() const override { return NodeType::FOR_LOOP; }
	
	void print() const override {
        std::cout << "For Loop Node:\n";
        std::cout << "Variable Declarations:\n";
        for (const auto& decl : var_decls) {
            decl->print();
        }
        if(condition_expr) {
            std::cout << "Condition: ";
            condition_expr->print();
        }
        std::cout << "Iteration:\n";
        for (const auto& iter : iterat) {
            iter->print();
        }
        if(block_statement) {
            std::cout << "Body:\n";
            block_statement->print();
        }
        std::cout << "End of For Loop Node\n";
	}
};

using for_loop = std::shared_ptr<ForLoopNode>;


struct BlockStatementNode : public Statement {
	std::vector<node> block_nodes;
	
	BlockStatementNode(const std::vector<node>& block_branches) : block_nodes(block_branches) {}	

	NodeType type() const override { return NodeType::BLOCK_STATEMENT; }

	void print() const override {
		std::cout << "Block Statement Node.\n";
		
		for (const auto& node : block_nodes) {
			node->print();
		}
	}
};

using block_st = std::shared_ptr<BlockStatementNode>;


struct JumpNode : public Statement {
	virtual ~JumpNode() = default;
	virtual NodeType type() const override = 0;
	virtual void print() const override = 0;	
};

using jump_node = std::shared_ptr<JumpNode>;


struct ReturnNode : public JumpNode {
	expr return_value;
	
	ReturnNode(const expr ret_value) : return_value(ret_value) {}
	
	NodeType type() const override { return NodeType::RETURN; }
	
	void print() const override {
        std::cout << "Return Statement Node:\n";
        if(return_value) {
            std::cout << "Value: ";
            return_value->print();
        }
        std::cout << "End of Return Statement Node\n";
    }
};

using return_node = std::shared_ptr<ReturnNode>;


struct BreakNode : public JumpNode {
	
	BreakNode() = default;
		
	NodeType type() const override { return NodeType::BREAK; }
	
	void print() const override {
		std::cout << "Break Node." << std::endl;
	}
};

using break_node = std::shared_ptr<BreakNode>;


struct ContinueNode : public JumpNode {
	ContinueNode() = default;
		
	NodeType type() const override { return NodeType::CONTINUE; }
	
	void print() const override {
		std::cout << "Continue Node." << std::endl;
	}
};

using continue_node = std::shared_ptr<ContinueNode>;


///////////// EXPRESSION ////////////////


struct BinaryNode : public Expression {
	std::string op;
	expr left_branch, right_branch;

	BinaryNode(const std::string& op, const expr& left_branch, const expr& right_branch)
		: op(op), left_branch(left_branch), right_branch(right_branch) {}
	
	NodeType type() const override { return NodeType::BINARY; }
	
	void print() const override {
        std::cout << "Binary Node:\n";
        if(left_branch) {
            std::cout << "Left: ";
            left_branch->print();
        }
        std::cout << "Operator: " << op << std::endl;
        if(right_branch) {
            std::cout << "Right: ";
            right_branch->print();
        }
        std::cout << "End of Binary Node\n";
    }
};

using bin_node = std::shared_ptr<BinaryNode>;


struct UnaryNode : public Expression {
	virtual ~UnaryNode() noexcept = default;
	virtual NodeType type() const override = 0;
	virtual void print() const override = 0;
};

using unary_node = std::shared_ptr<UnaryNode>;


struct Prefix_UnaryNode : public UnaryNode {
	std::string op;
	expr branch;
	
	Prefix_UnaryNode(const std::string& op, const expr branch) : op(op), branch(branch) {}
	
	NodeType type() const override { return NodeType::PREFIX_OP; }
	
	void print() const override {
        std::cout << "Prefix Unary Node:\n";
        std::cout << "Operator: " << op << std::endl;
        if(branch) {
            branch ->print();
        }
        std::cout << "End of Prefix Unary Node\n";
    }
};

using pref_unary_node = std::shared_ptr<Prefix_UnaryNode>;


struct Postfix_UnaryNode : public UnaryNode {
	std::string op;
	expr branch;
	
	Postfix_UnaryNode(const std::string& op, const expr branch) : op(op), branch(branch) {}
	
	NodeType type() const override { return NodeType::POSTFIX_OP; }

	void print() const override {
        std::cout << "Postfix Unary Node:\n";
        if(branch) {
            branch->print();
        }
        std::cout << "Operator: " << op << std::endl;
        std::cout << "End of Postfix Unary Node\n";
    }
};

using post_unary_node = std::shared_ptr<Postfix_UnaryNode>;


struct IdentifierNode : public Expression {
	std::string name;

	IdentifierNode(const std::string& name) : name(name) {}
	
	NodeType type() const override { return NodeType::IDENTIFIER; }
	
	void print() const override {
		std::cout << "Identifier Node. Identifier name: " << name << std::endl;
	}
};

using id_node = std::shared_ptr<IdentifierNode>;


struct FunctionNode : public Expression {
	std::string name;
	std::vector<expr> branches;

	FunctionNode(const std::string& name, const std::vector<expr>& branches)
		: name(name), branches(branches) {}
	
	NodeType type() const override { return NodeType::FUNCTION; }
	
	 void print() const override {
        std::cout << "Function Node:\n";
        std::cout << "Name: " << name << std::endl;
        std::cout << "Arguments:\n";
        for (const auto& arg : branches) {
            arg->print();
        }
        std::cout << "End of Function Node\n";
    }
};

using func_node = std::shared_ptr<FunctionNode>;


struct ParenthesizedNode : public Expression {
	expr _expr;

	ParenthesizedNode(const expr& _expr) : _expr(_expr) {}	
	NodeType type() const override { return NodeType::PARENTHES; }
	
	void print() const override {
        std::cout << "Parenthesized Node:\n";
        if(_expr) {
            _expr->print();
        }
        std::cout << "End of Parenthesized Node\n";
    }
}; 

using paren_node = std::shared_ptr<ParenthesizedNode>;


struct NumberNode : public Expression {
	double value;

	NumberNode(double value) : value(value) {}
	
	NodeType type() const override { return NodeType::NUMBER; }
	
	void print() const override {
		std::cout << "Number Node. Value: " << value << std::endl;
	}
};

using num_node = std::shared_ptr<NumberNode>;


struct CharacterNode : public Expression {
	char sym;

	CharacterNode(char sym) : sym(sym) {}
	
	NodeType type() const override { return NodeType::CHARACTER; }
	
	void print() const override {
		std::cout << "Character Node. Sym: " << sym << std::endl;
	}
};

using char_node = std::shared_ptr<CharacterNode>;


struct BoolNode : public Expression {
	bool value;

	BoolNode(bool value) : value(value) {}
	
	NodeType type() const override { return NodeType::BOOL; }
	
	void print() const override {
		std::cout << "Bool Node. Value: " << value << std::endl;
	}
};

using bool_node = std::shared_ptr<BoolNode>;


struct StringNode : public Expression {
	std::string _str;
	expr str_expr;

	StringNode(const std::string& _str, const expr str_expr = nullptr) : _str(_str), str_expr(str_expr) {}
	
	NodeType type() const override { return NodeType::STRING; }
	
	void print() const override {
		std::cout << "String Node. String: " << _str << std::endl;
		
		if (str_expr) str_expr->print();
	}
};

using str_node = std::shared_ptr<StringNode>;


struct ArrayNode : public Declaration {
    std::string name;
    std::vector<expr> elements;

    ArrayNode(const std::string& name, const std::vector<expr>& elements)
        : name(name), elements(elements) {}

    NodeType type() const override { return NodeType::ARRAY; }

    void print() const override {
        std::cout << "Array Declaration: " << name << " = { ";
        for (const auto& elem : elements) {
            elem->print();
            std::cout << ", ";
        }
        std::cout << "}\n";
    }
};

struct ArrayAccessNode : public Expression {
    expr array;
    expr index;

    ArrayAccessNode(expr array, expr index)
        : array(array), index(index) {}

    NodeType type() const override { return NodeType::ARRAY_ACCESS; }

    void print() const override {
        std::cout << "Array Access: ";
        array->print();
        std::cout << "[";
        index->print();
        std::cout << "]";
    }
};
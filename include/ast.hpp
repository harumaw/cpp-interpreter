/*
ASTNode
├── Declaration
│   ├── VariableDeclarationNode
│   ├── FunctionDeclarationNode
│
├── Statement
|   ├── BlockStatementNode
│   ├── IF_Statement_Node
│   ├── ELSE_Statement_Node
│   ├── ConditionalNode
│   ├── WhileLoopNode
│   ├── ForLoopNode
│   ├── ReturnStatementNode
│
└── Expression
    ├── BinaryNode
    ├── Prefix_UnaryNode
    ├── Postfix_UnaryNode
    ├── IdentifierNode
    ├── FunctionNode
    ├── ParenthesizedNode
    ├── NumberNode
    ├── CharacterNode
    ├── BoolNode
    ├── StringNode
*/

#pragma once


#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <memory>   


enum class NodeType{
    VARIABLE_DECLARATION, 
    FUNCTION_DECLARATION,


    BLOCK_STATEMENT,
    IF_STATEMENT,
    ELSE_STATEMENT,
    CONDITIONAL, 
    WHILE_L,
    FOR_L, 
    RETURN,
    BREAK,
    CONTINUE,

    BINARY, 
    PREFIX_OP,
    POSTFIX_OP,
    FUNCTION_CALL,
    ID,
    NUMBER,
    STRING,
    BOOL,
    CHARACTER,
    PARENTHES
};

struct ASTNode{
    virtual ~ASTNode() = default;
    virtual NodeType type() const = 0;
    virtual void print() const = 0;
};

using ast_node = std::shared_ptr<ASTNode>;



struct Declaration: public ASTNode{
    virtual ~Declaration() = default;
    virtual NodeType type() const override = 0;
    virtual void print() const override = 0;
};

using declaration = std::shared_ptr<Declaration>;

struct Statement: public ASTNode{
    virtual ~Statement() = default;
    virtual NodeType type() const override = 0;
    virtual void print() const override = 0;
};

using statement = std::shared_ptr<Statement>;

struct Expression: public ASTNode{
    virtual ~Expression() = default;
    virtual NodeType type() const override = 0;
    virtual void print() const override = 0;
};

using expression = std::shared_ptr<Expression>;

/// declaration
using decl_list = std::vector<std::pair<std::string, expression>>; // pair 1. name var 2. expr or value of var 
                                                                // "a", std::make_shared<NumberNode>(10) = a = 10

struct VariableDeclarationNode: public Declaration{
    std::string var_type;
    decl_list vars_list; // list of peremmennih

    VariableDeclarationNode(const std::string& var_type, const decl_list vars_list) : var_type(var_type), vars_list(vars_list) {}
    NodeType type() const override{
        return NodeType::VARIABLE_DECLARATION;
    }

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
    decl_list args; 
    statement body;  

    FunctionDeclarationNode(const std::string& function_type, const std::string& function_name, 
                            const decl_list& args, const statement& body) 
        : function_type(function_type), function_name(function_name), args(args), body(body) {}

    NodeType type() const override {
        return NodeType::FUNCTION_DECLARATION;
    }

    void print() const override {
        std::cout << "Function Declaration Node.\n";
        std::cout << "Type: " << function_type << ", Function name: " << function_name << std::endl;
        std::cout << "Arguments: ";
        for (const auto& [arg_name, arg_value] : args) {
            std::cout << arg_name;
            if (arg_value) {
                std::cout << " = ";
                arg_value->print();
            }
            std::cout << ", ";
        }
        std::cout << "\nBody:\n";
        if (body) {
            body->print();
        }
        std::cout << "End of Function Declaration Node\n" << std::endl;
    }
};
using func_decl = std::shared_ptr<FunctionDeclarationNode>; 





struct BlockStatementNode : public Statement{
    std::vector<statement> statements;
    
    BlockStatementNode(const std::vector<statement>& statements) : statements(statements) {}

    NodeType type() const override {
        return NodeType::BLOCK_STATEMENT;
    }
    void print() const override {
        std::cout << "Block Statement Node:\n";
        for (const auto& stmt : statements) {
            stmt->print();
        }
        std::cout << "End of Block Statement Node\n";
    }
};
using block_stmt = std::shared_ptr<BlockStatementNode>;


struct IfStatementNode : public Statement{
    expression condition;
    statement if_body;

    IfStatementNode(const expression& condition, const statement& if_body) : condition(condition), if_body(if_body) {}  

    NodeType type() const override {
        return NodeType::IF_STATEMENT;
    }
    
    void print() const override {
        std::cout << "If Statement Node:\n";
        if(condition) {
            std::cout << "Condition: ";
            condition->print();
        }
        if(if_body) {
            std::cout << "If Body:\n";
            if_body->print();
        }

        std::cout << "End of If Statement Node\n";
    }


};
using if_statement = std::shared_ptr<IfStatementNode>;


struct ElseStatementNode: public Statement{
    statement else_body;

    ElseStatementNode(const statement& else_body) : else_body(else_body) {}

    NodeType type() const override {
        return NodeType::ELSE_STATEMENT;
    }

    void print() const override {
        std::cout << "Else Statement Node:\n";
        if(else_body) {
            std::cout << "Else Body:\n";
            else_body->print();
        }
        std::cout << "End of Else Statement Node\n";
    }
};
using else_statement = std::shared_ptr<ElseStatementNode>;

struct ConditionalNode: public Statement{
    statement if_statement;
    statement else_statement;

    ConditionalNode(const statement& if_statement, const statement& else_statement) 
        : if_statement(if_statement), else_statement(else_statement) {}

    NodeType type() const override {
        return NodeType::CONDITIONAL;
    }   

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
using conditional = std::shared_ptr<ConditionalNode>;


struct LoopNode : public Statement {
	virtual ~LoopNode() = default;
	virtual NodeType type() const override = 0;
	virtual void print() const override = 0;
};

using loop = std::shared_ptr<LoopNode>;

struct WhileLoopNode : public LoopNode{
    expression condition;
    statement body;

    WhileLoopNode(const expression& condition, const statement& body) : condition(condition), body(body) {}

    NodeType type() const override {
        return NodeType::WHILE_L;
    }

    void print() const override {
        std::cout << "While Loop Node:\n";
        if(condition) {
            std::cout << "Condition: ";
            condition->print();
        }
        if(body) {
            std::cout << "Body:\n";
            body->print();
        }
        std::cout << "End of While Loop Node\n";
    }

};
using while_loop = std::shared_ptr<WhileLoopNode>;

struct ForLoopNode : public LoopNode{
    std::vector<declaration> var_decls;
    expression condition;
    std::vector<expression> iterat;
    statement body;

    ForLoopNode(const std::vector<declaration>& var_decls, const expression& condition, 
                const std::vector<expression>& iterat, const statement& body) 
        : var_decls(var_decls), condition(condition), iterat(iterat), body(body) {}

    NodeType type() const override {
        return NodeType::FOR_L;
    }
    void print() const override {
        std::cout << "For Loop Node:\n";
        std::cout << "Variable Declarations:\n";
        for (const auto& decl : var_decls) {
            decl->print();
        }
        if(condition) {
            std::cout << "Condition: ";
            condition->print();
        }
        std::cout << "Iteration:\n";
        for (const auto& iter : iterat) {
            iter->print();
        }
        if(body) {
            std::cout << "Body:\n";
            body->print();
        }
        std::cout << "End of For Loop Node\n";
    }
};
using for_loop = std::shared_ptr<ForLoopNode>;

struct JumpNode : public Statement {
    virtual ~JumpNode() = default;
    virtual NodeType type() const override = 0;
    virtual void print() const override = 0;
};
using jump = std::shared_ptr<JumpNode>;

struct ReturnStatementNode : public JumpNode {
    expression return_value;

    ReturnStatementNode(const expression& value) : return_value(value) {}

    NodeType type() const override {
        return NodeType::RETURN;
    }

    void print() const override {
        std::cout << "Return Statement Node:\n";
        if(return_value) {
            std::cout << "Value: ";
            return_value->print();
        }
        std::cout << "End of Return Statement Node\n";
    }
};  
using return_jumpnode = std::shared_ptr<ReturnStatementNode>;

struct BreakStatementNode : public JumpNode {
    BreakStatementNode() = default;
    NodeType type() const override {
        return NodeType::BREAK;
    }

    void print() const override {
        std::cout << "Break Statement Node\n";
    }
};
using break_jumpnode = std::shared_ptr<BreakStatementNode>;

struct ContinueStatementNode : public JumpNode {
    ContinueStatementNode() = default;
    NodeType type() const override {
        return NodeType::CONTINUE;
    }

    void print() const override {
        std::cout << "Continue Statement Node\n";
    }
};
using continue_jumpnode = std::shared_ptr<ContinueStatementNode>;

struct BinaryNode: public Expression{
    std::string op;
    expression left;
    expression right;

    BinaryNode(const std::string& op, const expression& left, const expression& right) : op(op), left(left), right(right) {}

    NodeType type() const override {
        return NodeType::BINARY;
    }

    void print() const override {
        std::cout << "Binary Node:\n";
        if(left) {
            std::cout << "Left: ";
            left->print();
        }
        std::cout << "Operator: " << op << std::endl;
        if(right) {
            std::cout << "Right: ";
            right->print();
        }
        std::cout << "End of Binary Node\n";
    }
};
using binary_node = std::shared_ptr<BinaryNode>;

struct UnaryNode : public Expression{
    virtual ~UnaryNode() = default;
    virtual NodeType type() const override = 0;
    virtual void print() const override = 0;
};
using unary_node = std::shared_ptr<UnaryNode>;

struct PrefixUnaryNode : public UnaryNode {
    std::string op;
    expression branch;

    PrefixUnaryNode(const std::string& op, const expression& branch) : op(op), branch(branch) {}

    NodeType type() const override {
        return NodeType::PREFIX_OP;
    }

    void print() const override {
        std::cout << "Prefix Unary Node:\n";
        std::cout << "Operator: " << op << std::endl;
        if(branch) {
            branch ->print();
        }
        std::cout << "End of Prefix Unary Node\n";
    }
};
using prefix_unary = std::shared_ptr<PrefixUnaryNode>;

struct PostfixUnaryNode : public UnaryNode {
    std::string op;
    expression branch;

    PostfixUnaryNode(const std::string& op, const expression& branch) : op(op), branch(branch) {}

    NodeType type() const override {
        return NodeType::POSTFIX_OP;
    }

    void print() const override {
        std::cout << "Postfix Unary Node:\n";
        if(branch) {
            branch->print();
        }
        std::cout << "Operator: " << op << std::endl;
        std::cout << "End of Postfix Unary Node\n";
    }
};
using postfix_unary = std::shared_ptr<PostfixUnaryNode>;

struct IdentifierNode : public Expression {
    std::string name;

    IdentifierNode(const std::string& name) : name(name) {}

    NodeType type() const override {
        return NodeType::ID;
    }

    void print() const override {
        std::cout << "Identifier Node: " << name << std::endl;
    }
};
using identifier = std::shared_ptr<IdentifierNode>;

struct FunctionNode : public Expression {
    std::string name;
    std::vector<expression> args;

    FunctionNode(const std::string& name, const std::vector<expression>& args) : name(name), args(args) {}

    NodeType type() const override {
        return NodeType::FUNCTION_CALL;
    }

    void print() const override {
        std::cout << "Function Node:\n";
        std::cout << "Name: " << name << std::endl;
        std::cout << "Arguments:\n";
        for (const auto& arg : args) {
            arg->print();
        }
        std::cout << "End of Function Node\n";
    }
};
using function_call = std::shared_ptr<FunctionNode>;

struct ParenthesizedNode : public Expression {
    expression expr;

    ParenthesizedNode(const expression& expr) : expr(expr) {}

    NodeType type() const override {
        return NodeType::PARENTHES;
    }

    void print() const override {
        std::cout << "Parenthesized Node:\n";
        if(expr) {
            expr->print();
        }
        std::cout << "End of Parenthesized Node\n";
    }
};
using parenthesized = std::shared_ptr<ParenthesizedNode>;


struct NumberNode : public Expression {
    double value;

    NumberNode(double value) : value(value) {}

    NodeType type() const override {
        return NodeType::NUMBER;
    }

    void print() const override {
        std::cout << "Number Node: " << value << std::endl;
    }
};
using number_node = std::shared_ptr<NumberNode>;

struct StringNode : public Expression {
    std::string value;

    StringNode(const std::string& value) : value(value) {}

    NodeType type() const override {
        return NodeType::STRING;
    }

    void print() const override {
        std::cout << "String Node: " << value << std::endl;
    }
};
using string_node = std::shared_ptr<StringNode>;

struct BoolNode : public Expression {
    bool value;

    BoolNode(bool value) : value(value) {}

    NodeType type() const override {
        return NodeType::BOOL;
    }

    void print() const override {
        std::cout << "Bool Node: " << std::boolalpha << value << std::endl;
    }
};
using bool_node = std::shared_ptr<BoolNode>;


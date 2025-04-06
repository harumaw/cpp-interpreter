#include <iostream>
#include "printer.hpp"

void Printer::visit(RootNode& node) {
    std::cout << "RootNode { \n" << std::endl;
    for (auto& decl : node.declarations) {
        decl->accept(*this);
        std::cout << std::endl;
    }
    std::cout << "}" << std::endl;
}

void Printer::visit(Declaration::NoPtrDeclarator& node) {
    std::cout << "NoPtrDeclarator: \n" << node.name;
}

void Printer::visit(Declaration::PtrDeclarator& node) {
    std::cout << "PtrDeclarator: \n*" << node.name;
}

void Printer::visit(Declaration::InitDeclarator& node) {
    std::cout << "InitDeclarator: \n";
    node.declarator->accept(*this);
    if (node.initializer) {
        std::cout << " = ";
        node.initializer->accept(*this);
    }
}

void Printer::visit(VarDeclaration& node) {
    std::cout << "VarDeclaration: \n" << node.type << " ";
    for (std::size_t i = 0, size = node.declarator_list.size(); i < size; ++i) {
        node.declarator_list[i]->accept(*this);
        if (i != size - 1) {
            std::cout << ", ";
        }
    }
    std::cout << ";";
}

void Printer::visit(ParameterDeclaration& node) {
    std::cout << "ParameterDeclaration:  \n" << node.type << " ";
    node.init_declarator->accept(*this);
}

void Printer::visit(FuncDeclaration& node) {
    std::cout << "FuncDeclaration: \n " << node.type << " ";
    node.declarator->accept(*this);
    std::cout << "(";
    for (std::size_t i = 0, size = node.args.size(); i < size; ++i) {
        node.args[i]->accept(*this);
        if (i != size - 1) {
            std::cout << ", ";
        }
    }
    std::cout << ")";
    if (node.body) {
        node.body->accept(*this);
    } else {
        std::cout << ";";
    }
}

void Printer::visit(CompoundStatement& node) {
    std::cout << "CompoundStatement { \n" << std::endl;
    for (auto& statement : node.statements) {
        statement->accept(*this);
        std::cout << std::endl;
    }
    std::cout << "}";
}

void Printer::visit(DeclarationStatement& node) {
    std::cout << "DeclarationStatement: \n";
    node.declaration->accept(*this);
}

void Printer::visit(ExpressionStatement& node) {
    std::cout << "ExpressionStatement: \n";
    node.expression->accept(*this);
    std::cout << ";";
}

void Printer::visit(ConditionalStatement& node) {
    std::cout << "ConditionalStatement: if ( \n";
    node.if_branch.first->accept(*this);
    std::cout << ") ";
    node.if_branch.second->accept(*this);
    if (node.else_branch) {
        std::cout << " else ";
        node.else_branch->accept(*this);
    }
}

void Printer::visit(WhileStatement& node) {
    std::cout << "WhileStatement: while ( \n";
    node.condition->accept(*this);
    std::cout << ") ";
    node.statement->accept(*this);
}

void Printer::visit(ForStatement& node) {
    std::cout << "ForStatement: for (";

    if (node.initialization) {
        node.initialization->accept(*this);
    }
    std::cout << "; ";

    if (node.condition) {
        node.condition->accept(*this);
    }
    std::cout << "; ";

    if (node.increment) {
        node.increment->accept(*this);
    }
    std::cout << ") ";

    if (node.body) {
        node.body->accept(*this);
    }
}
void Printer::visit(ReturnStatement& node) {
    std::cout << "ReturnStatement: return \n";
    node.expression->accept(*this);
    std::cout << ";";
}

void Printer::visit(BreakStatement&) {
    std::cout << "BreakStatement: break; \n";
}

void Printer::visit(ContinueStatement&) {
    std::cout << "ContinueStatement: continue; \n";
}

void Printer::visit(BinaryOperation& node) {
    std::cout << "BinaryOperation: \n (";
    node.lhs->accept(*this);
    std::cout << " " << node.op << " ";
    node.rhs->accept(*this);
    std::cout << ")";
}

void Printer::visit(PrefixExpression& node) {
    std::cout << "PrefixExpression: \n" << node.op;
    node.base->accept(*this);
}

void Printer::visit(PostfixIncrementExpression& node) {
    std::cout << "PostfixIncrementExpression: \n";
    node.base->accept(*this);
    std::cout << "++";
}

void Printer::visit(PostfixDecrementExpression& node) {
    std::cout << "PostfixDecrementExpression: \n";
    node.base->accept(*this);
    std::cout << "--";
}

void Printer::visit(SubscriptExpression& node) {
    std::cout << "SubscriptExpression: \n";
    node.base->accept(*this);
    std::cout << "[";
    node.index->accept(*this);
    std::cout << "]";
}

void Printer::visit(FunctionCallExpression& node) {
    std::cout << "FunctionCallExpression: \n";
    node.base->accept(*this);
    std::cout << "(";
    for (std::size_t i = 0, size = node.args.size(); i < size; ++i) {
        node.args[i]->accept(*this);
        if (i != size - 1) {
            std::cout << ", ";
        }
    }
    std::cout << ")";
}

void Printer::visit(IdentifierExpression& node) {
    std::cout << "IdentifierExpression: \n" << node.name;
}

void Printer::visit(IntLiteral& node) {
    std::cout << "IntLiteral: \n" << node.value;
}

void Printer::visit(FloatLiteral& node) {
    std::cout << "FloatLiteral: \n" << node.value;
}

void Printer::visit(CharLiteral& node) {
    std::cout << "CharLiteral: \n '" << node.value << "'";
}

void Printer::visit(StringLiteral& node) {
    std::cout << "StringLiteral:  \n \"" << node.value << "\"";
}

void Printer::visit(BoolLiteral& node) {
    std::cout << "BoolLiteral: \n" << (node.value ? "true" : "false");
}

void Printer::visit(ParenthesizedExpression& node) {
    std::cout << "ParenthesizedExpression: \n (";
    node.expression->accept(*this);
    std::cout << ")";
}
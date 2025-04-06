#include <iostream>
#include "printer.hpp"

void Printer::visit(TranslationUnit& node) {
    std::cout << "TranslationUnit {" << std::endl;
    for (auto& decl : node.declarations) {
        decl->accept(*this);
        std::cout << std::endl;
    }
    std::cout << "}" << std::endl;
}

void Printer::visit(Declaration::NoPtrDeclarator& node) {
    std::cout << "NoPtrDeclarator: " << node.name;
}

void Printer::visit(Declaration::PtrDeclarator& node) {
    std::cout << "PtrDeclarator: *" << node.name;
}

void Printer::visit(Declaration::InitDeclarator& node) {
    std::cout << "InitDeclarator: ";
    node.declarator->accept(*this);
    if (node.initializer) {
        std::cout << " = ";
        node.initializer->accept(*this);
    }
}

void Printer::visit(VarDeclaration& node) {
    std::cout << "VarDeclaration: " << node.type << " ";
    for (std::size_t i = 0, size = node.declarator_list.size(); i < size; ++i) {
        node.declarator_list[i]->accept(*this);
        if (i != size - 1) {
            std::cout << ", ";
        }
    }
    std::cout << ";";
}

void Printer::visit(ParameterDeclaration& node) {
    std::cout << "ParameterDeclaration: " << node.type << " ";
    node.init_declarator->accept(*this);
}

void Printer::visit(FuncDeclaration& node) {
    std::cout << "FuncDeclaration: " << node.type << " ";
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
    std::cout << "CompoundStatement {" << std::endl;
    for (auto& statement : node.statements) {
        statement->accept(*this);
        std::cout << std::endl;
    }
    std::cout << "}";
}

void Printer::visit(DeclarationStatement& node) {
    std::cout << "DeclarationStatement: ";
    node.declaration->accept(*this);
}

void Printer::visit(ExpressionStatement& node) {
    std::cout << "ExpressionStatement: ";
    node.expression->accept(*this);
    std::cout << ";";
}

void Printer::visit(ConditionalStatement& node) {
    std::cout << "ConditionalStatement: if (";
    node.if_branch.first->accept(*this);
    std::cout << ") ";
    node.if_branch.second->accept(*this);
    if (node.else_branch) {
        std::cout << " else ";
        node.else_branch->accept(*this);
    }
}

void Printer::visit(WhileStatement& node) {
    std::cout << "WhileStatement: while (";
    node.condition->accept(*this);
    std::cout << ") ";
    node.statement->accept(*this);
}

void Printer::visit(ForStatement&) {
    std::cout << "ForStatement: TBD";
}

void Printer::visit(RepeatStatement& node) {
    std::cout << "RepeatStatement: repeat ";
    node.statement->accept(*this);
}

void Printer::visit(ReturnStatement& node) {
    std::cout << "ReturnStatement: return ";
    node.expression->accept(*this);
    std::cout << ";";
}

void Printer::visit(BreakStatement&) {
    std::cout << "BreakStatement: break;";
}

void Printer::visit(ContinueStatement&) {
    std::cout << "ContinueStatement: continue;";
}

void Printer::visit(BinaryOperation& node) {
    std::cout << "BinaryOperation: (";
    node.lhs->accept(*this);
    std::cout << " " << node.op << " ";
    node.rhs->accept(*this);
    std::cout << ")";
}

void Printer::visit(PrefixExpression& node) {
    std::cout << "PrefixExpression: " << node.op;
    node.base->accept(*this);
}

void Printer::visit(PostfixIncrementExpression& node) {
    std::cout << "PostfixIncrementExpression: ";
    node.base->accept(*this);
    std::cout << "++";
}

void Printer::visit(PostfixDecrementExpression& node) {
    std::cout << "PostfixDecrementExpression: ";
    node.base->accept(*this);
    std::cout << "--";
}

void Printer::visit(SubscriptExpression& node) {
    std::cout << "SubscriptExpression: ";
    node.base->accept(*this);
    std::cout << "[";
    node.index->accept(*this);
    std::cout << "]";
}

void Printer::visit(FunctionCallExpression& node) {
    std::cout << "FunctionCallExpression: ";
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
    std::cout << "IdentifierExpression: " << node.name;
}

void Printer::visit(IntLiteral& node) {
    std::cout << "IntLiteral: " << node.value;
}

void Printer::visit(FloatLiteral& node) {
    std::cout << "FloatLiteral: " << node.value;
}

void Printer::visit(CharLiteral& node) {
    std::cout << "CharLiteral: '" << node.value << "'";
}

void Printer::visit(StringLiteral& node) {
    std::cout << "StringLiteral: \"" << node.value << "\"";
}

void Printer::visit(BoolLiteral& node) {
    std::cout << "BoolLiteral: " << (node.value ? "true" : "false");
}

void Printer::visit(ParenthesizedExpression& node) {
    std::cout << "ParenthesizedExpression: (";
    node.expression->accept(*this);
    std::cout << ")";
}
#include <iostream>
#include <string>
#include "printer.hpp"

void Printer::indent() {
    for (int i = 0; i < indent_level; ++i) {
        std::cout << "  ";
    }
}

void Printer::visit(TranslationUnit& node) {
    std::cout << "TranslationUnit {\n";
    ++indent_level;
    for (auto& decl : node.declarations) {
        decl->accept(*this);
        std::cout << "\n";
    }
    --indent_level;
    std::cout << "}\n";
}


void Printer::visit(ASTNode& node) {
    std::cout << "ASTNODE {\n";
    std::cout << "}\n";
}




void Printer::visit(Declaration::SimpleDeclarator& node) {
    indent();
    std::cout << "SimpleDeclarator: " << node.name << "\n";
}

void Printer::visit(Declaration::PtrDeclarator& node) {
    indent();
    std::cout << "PtrDeclarator: *\n";
    ++indent_level;
    if (node.inner) {
        node.inner->accept(*this);
    }
    --indent_level;
}
void Printer::visit(Declaration::InitDeclarator& node) {
    indent();
    std::cout << "InitDeclarator:\n";
    ++indent_level;
    node.declarator->accept(*this);
    if (node.initializer) {
        indent();
        std::cout << "= \n";
        ++indent_level;
        node.initializer->accept(*this);
        --indent_level;
    }
    --indent_level;
}

void Printer::visit(VarDeclaration& node) {
    indent();
    std::cout << "VarDeclaration: "
              << (node.is_const ? "const " : "")
              << node.type << "\n";

    ++indent_level;
    for (std::size_t i = 0; i < node.declarator_list.size(); ++i) {
        node.declarator_list[i]->accept(*this);
    }
    --indent_level;
}

void Printer::visit(ParameterDeclaration& node) {
    indent();
    std::cout << "ParameterDeclaration: " << node.type << "\n";
    ++indent_level;
    node.init_declarator->accept(*this);
    --indent_level;
}

void Printer::visit(FuncDeclaration& node) {
    indent();
    std::cout << "FuncDeclaration: "
              << (node.is_const ? "const " : "") << node.type << "\n";
    std::cout<< (node.is_readonly ? "readonly " : "");
    ++indent_level;
    node.declarator->accept(*this);
    std::cout << "\n";
    indent();
    std::cout << "Parameters:\n";
    ++indent_level;
    for (auto& arg : node.args) {
        arg->accept(*this);
    }

    
    --indent_level;
    if (node.body) {
        node.body->accept(*this);
    } else {
        indent();
        std::cout << ";\n";
    }
    --indent_level;
}

void Printer::visit(CompoundStatement& node) {
    indent();
    std::cout << "CompoundStatement {\n";
    ++indent_level;
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
    --indent_level;
    indent();
    std::cout << "}\n";
}

void Printer::visit(DeclarationStatement& node) {
    indent();
    std::cout << "DeclarationStatement:\n";
    ++indent_level;
    node.declaration->accept(*this);
    --indent_level;
}

void Printer::visit(ExpressionStatement& node) {
    indent();
    std::cout << "ExpressionStatement:\n";
    ++indent_level;
    node.expression->accept(*this);
    --indent_level;
}

void Printer::visit(ConditionalStatement& node) {
    indent();
    std::cout << "ConditionalStatement: if\n";
    ++indent_level;
    indent();
    std::cout << "Condition:\n";
    ++indent_level;
    node.if_branch.first->accept(*this);
    --indent_level;
    indent();
    std::cout << "Then:\n";
    ++indent_level;
    node.if_branch.second->accept(*this);
    --indent_level;
    if (node.else_branch) {
        indent();
        std::cout << "Else:\n";
        ++indent_level;
        node.else_branch->accept(*this);
        --indent_level;
    }
    --indent_level;
}

void Printer::visit(WhileStatement& node) {
    indent();
    std::cout << "WhileStatement:\n";
    ++indent_level;
    indent();
    std::cout << "Condition:\n";
    ++indent_level;
    node.condition->accept(*this);
    --indent_level;
    indent();
    std::cout << "Body:\n";
    ++indent_level;
    node.statement->accept(*this);
    --indent_level;
    --indent_level;
}

void Printer::visit(ForStatement& node) {
    indent();
    std::cout << "ForStatement:\n";
    ++indent_level;

    if (node.initialization) {
        indent();
        std::cout << "Initialization:\n";
        ++indent_level;
        node.initialization->accept(*this);
        --indent_level;
    }

    if (node.condition) {
        indent();
        std::cout << "Condition:\n";
        ++indent_level;
        node.condition->accept(*this);
        --indent_level;
    }

    if (node.increment) {
        indent();
        std::cout << "Increment:\n";
        ++indent_level;
        node.increment->accept(*this);
        --indent_level;
    }

    if (node.body) {
        indent();
        std::cout << "Body:\n";
        ++indent_level;
        node.body->accept(*this);
        --indent_level;
    }

    --indent_level;
}

void Printer::visit(ReturnStatement& node) {
    indent();
    std::cout << "ReturnStatement:\n";
    ++indent_level;
    if(node.expression) {
        node.expression->accept(*this);
    } else {
        indent();
        std::cout << ";\n";
    }
    --indent_level;
}

void Printer::visit(BreakStatement&) {
    indent();
    std::cout << "BreakStatement\n";
}

void Printer::visit(ContinueStatement&) {
    indent();
    std::cout << "ContinueStatement\n";
}

void Printer::visit(BinaryOperation& node) {
    indent();
    std::cout << "BinaryOperation: " << node.op << "\n";
    ++indent_level;
    node.lhs->accept(*this);
    node.rhs->accept(*this);
    --indent_level;
}

void Printer::visit(PrefixExpression& node) {
    indent();
    std::cout << "PrefixExpression: " << node.op << "\n";
    ++indent_level;
    node.base->accept(*this);
    --indent_level;
}

void Printer::visit(PostfixIncrementExpression& node) {
    indent();
    std::cout << "PostfixIncrementExpression: ++\n";
    ++indent_level;
    node.base->accept(*this);
    --indent_level;
}

void Printer::visit(PostfixDecrementExpression& node) {
    indent();
    std::cout << "PostfixDecrementExpression: --\n";
    ++indent_level;
    node.base->accept(*this);
    --indent_level;
}


void Printer::visit(FunctionCallExpression& node) {
    indent();
    std::cout << "FunctionCallExpression:\n";
    ++indent_level;
    node.base->accept(*this);
    for (auto& arg : node.args) {
        arg->accept(*this);
    }
    --indent_level;
}

void Printer::visit(IdentifierExpression& node) {
    indent();
    std::cout << "Identifier: " << node.name << "\n";
}

void Printer::visit(IntLiteral& node) {
    indent();
    std::cout << "IntLiteral: " << node.value << "\n";
}

void Printer::visit(FloatLiteral& node) {
    indent();
    std::cout << "FloatLiteral: " << node.value << "\n";
}

void Printer::visit(CharLiteral& node) {
    indent();
    std::cout << "CharLiteral: '" << node.value << "'\n";
}

void Printer::visit(StringLiteral& node) {
    indent();
    std::cout << "StringLiteral: \"" << node.value << "\"\n";
}

void Printer::visit(BoolLiteral& node) {
    indent();
    std::cout << "BoolLiteral: " << (node.value ? "true" : "false") << "\n";
}

void Printer::visit(NullPtrLiteral&) {
    indent();
    std::cout << "NullPtrLiteral\n";
}

void Printer::visit(ParenthesizedExpression& node) {
    indent();
    std::cout << "ParenthesizedExpression:\n";
    ++indent_level;
    node.expression->accept(*this);
    --indent_level;
}

void Printer::visit(StructDeclaration& node) {
    indent();
    std::cout << "StructDeclaration: " << node.name << "\n";
    ++indent_level;
    for (auto& member : node.members) { 
        member->accept(*this);
    }
    --indent_level;
}


void Printer::visit(StructMemberAccessExpression& node) {
    indent();
    std::cout << "StructMemberAccessExpression:\n";
    ++indent_level;
    node.base->accept(*this);  
    indent();
    std::cout << "Member: " << node.member << "\n";
    --indent_level;
}

void Printer::visit(ArrayDeclaration &node) {
    indent();
    std::cout << "ArrayDeclaration:\n";

    ++indent_level;

    indent();
    std::cout << "Type: " << node.type << "\n";


    indent();
    std::cout << "Name: " << node.name << "\n";


    indent();
    std::cout << "Size: ";
    if (node.size) {
        node.size->accept(*this);
    } else {
        std::cout << "<unspecified>\n";
    }

    if (!node.initializer_list.empty()) {
        indent();
        std::cout << "Initializers:\n";
        ++indent_level;
        for (size_t i = 0; i < node.initializer_list.size(); ++i) {
            indent();
            std::cout << "[" << i << "]: ";
            node.initializer_list[i]->accept(*this);
        }
        --indent_level;
    }

    --indent_level;
}

void Printer::visit(SubscriptExpression &node) {
    indent();
    std::cout << "SubscriptExpression:\n";
    
    ++indent_level;
    
    indent();
    std::cout << "Base: ";
    node.base->accept(*this); 
    
    indent();
    std::cout << "Index: ";
    node.index->accept(*this); 
    
    --indent_level;
    }


void Printer::visit(TernaryExpression &node) {
    indent();
    std::cout << "TernaryExpression:\n";

    ++indent_level;
    
    indent();
    std::cout << "Condition: ";
    node.condition->accept(*this);

    indent();
    std::cout << "TrueExpr: ";
    node.true_expr->accept(*this);

    indent();
    std::cout << "FalseExpr: ";
    node.false_expr->accept(*this);

    --indent_level;
}

void Printer::visit(SizeOfExpression& node) {
    indent();
    std::cout << "SizeOfExpression:\n";
    
    ++indent_level;
    
    if (node.is_type) {
        indent();
        std::cout << "Type: " << node.type_name << "\n";
    } else {
        indent();
        std::cout << "Expression:\n";
        ++indent_level;
        node.expression->accept(*this);
        --indent_level;
    }

    --indent_level;
}

void Printer::visit(DoWhileStatement& node) {
    indent();
    std::cout << "DoWhileStatement:\n";
    
    ++indent_level;
    
    indent();
    std::cout << "Body:\n";
    ++indent_level;
    node.statement->accept(*this);
    --indent_level;

    indent();
    std::cout << "Condition:\n";
    ++indent_level;
    node.condition->accept(*this);
    --indent_level;

    --indent_level;
}

void Printer::visit(NameSpaceDeclaration& node) {
    indent();
    std::cout << "Namespace: " << node.name << "\n";
    ++indent_level;
    for (auto& decl : node.declarations) {
        decl->accept(*this);
    }
    --indent_level;
}

void Printer::visit(NameSpaceAcceptExpression& node) {
    indent();
    std::cout << "NameSpaceAcceptExpression:\n";
    
    ++indent_level;
    
    indent();
    std::cout << "Base: ";
    node.base->accept(*this); 
    
    indent();
    std::cout << "Name: " << node.name << "\n"; 
    
    --indent_level;
}

void Printer::visit(StaticAssertStatement& node){
    indent();
    std::cout << "StaticAssertStatement:\n";

    ++indent_level;

    indent();
    std::cout << "Condition";
    node.condition->accept(*this);

    indent();
    std::cout << "Message:" << node.msg << "\n";

    --indent_level;
}
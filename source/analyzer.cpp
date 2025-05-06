#include "analyzer.hpp"
#include "token.hpp"
#include <memory>


std::unordered_map<std::string, Type> Analyzer::default_types = {
    {"int", IntegerType()},    
    {"float", FloatType()},   
    {"double", FloatType()},  
    {"char", CharType()},      
    {"bool", BoolType()}       
};

Analyzer::Analyzer() : scope(nullptr) {}

void Analyzer::analyze(TranslationUnit& unit) {
    for (auto& node : unit.get_nodes()) {
        this->visit(*node); 
    }
}

void Analyzer::visit(ASTNode& node){
    node.accept(*this);
}

/*void Analyzer::visit(VarDeclaration& var_decl){
    auto type = var_decl.type;
    auto declarations = var_decl.declarator_list;

    if()
}*/

void Analyzer::visit(Declaration::PtrDeclarator& node) {
    // Обработка указателя-декларатора
    current_type = PointerType(get_type(node.name)); 
}

void Analyzer::visit(Declaration::SimpleDeclarator& node) {
    current_type = get_type(node.name);
}

void Analyzer::visit(Declaration::InitDeclarator& node) {
    if (auto ptr_declarator = dynamic_cast<Declaration::PtrDeclarator*>(node.declarator.get())) {
        this->visit(*ptr_declarator); 
    } else if (auto simple_declarator = dynamic_cast<Declaration::SimpleDeclarator*>(node.declarator.get())) {
        this->visit(*simple_declarator); 
    } else {
        throw std::runtime_error("Unsupported declarator type");
    }
    if (node.initializer) {
        this->visit(*node.initializer); 
    }
}

void Analyzer::visit(VarDeclaration& node) {
    auto type = get_type(node.type); 
    for (auto& declarator : node.declarator_list) {
        this->visit(*declarator); 
        scope->push_variable(declarator->declarator->name, type);
    }
}

void Analyzer::visit(ParameterDeclaration& node) {
    auto type = get_type(node.type); 
    this->visit(*node.init_declarator); 
    scope->push_variable(node.init_declarator->declarator->name, type);
}

void Analyzer::visit(FuncDeclaration& node) {
    auto return_type = get_type(node.type);
    std::vector<Type> arg_types;
    scope = scope->create_new_table(scope, node.body); 
    for (auto& param : node.args) {
        this->visit(*param); 
        arg_types.push_back(current_type); 
    }
    auto func_type = FuncType(return_type, arg_types); 
    scope->push_func(node.declarator->name, func_type); 
    this->visit(*node.body);
    scope = scope->get_prev_table(); 
    current_type = func_type; 
}

void Analyzer::visit(StructDeclaration& node) {
    std::unordered_map<std::string, Type> members;
    for (auto& member : node.members) {
        this->visit(*member); 
        members[member->declarator_list[0]->declarator->name] = current_type; 
    }
    auto struct_type = StructType(members); 
    scope->push_struct(node.name, struct_type); 
    current_type = struct_type;
}

void Analyzer::visit(ArrayDeclaration& node) {
    auto element_type = get_type(node.type); 
    this->visit(*node.size);
    current_type = ArrayType(element_type, node.size);
}

void Analyzer::visit(CompoundStatement& node) {
    scope = scope->create_new_table(scope, std::make_shared<CompoundStatement>(node)); 
    for (auto& stmt : node.statements) {
        this->visit(*stmt); 
    }
    scope = scope->get_prev_table(); 
}

void Analyzer::visit(DeclarationStatement& node) {
    this->visit(*node.declaration); 
}

void Analyzer::visit(ExpressionStatement& node) {
    this->visit(*node.expression);
}


void Analyzer::visit(ConditionalStatement& node) {
    this->visit(*(node.if_branch.first)); 
    this->visit(*(node.if_branch.second));
    if (node.else_branch) {
        this->visit(*node.else_branch); 
    }
}

void Analyzer::visit(WhileStatement& node) {
    this->visit(*node.condition);
    this->visit(*node.statement); 
}

void Analyzer::visit(ForStatement& node) {
    if (node.initialization) {
        this->visit(*node.initialization); 
    }
    if (node.condition) {
        this->visit(*node.condition); 
    }
    if (node.increment) {
        this->visit(*node.increment);
    }
    this->visit(*node.body);
}

void Analyzer::visit(ReturnStatement& node) {
    if (node.expression) {
        this->visit(*node.expression); 
    }
}

void Analyzer::visit(BreakStatement& node) {

}

void Analyzer::visit(ContinueStatement& node) {
}

void Analyzer::visit(StructMemberAccessExpression& node) {
    this->visit(*node.base); 
    auto struct_type = dynamic_cast<StructType*>(&current_type);
    if (!struct_type) {
        throw std::runtime_error("Expression is not a struct");
    }
    if (struct_type->members.find(node.member) == struct_type->members.end()) {
        throw std::runtime_error("Struct has no member named '" + node.member + "'");
    }
    current_type = struct_type->members[node.member];
}



Type Analyzer::get_type(const std::string& type_name) {
    try {
        return scope->match_struct(type_name);
    } catch (const std::runtime_error&) {
        return default_types.at(type_name);
    }
}



/*
1. dodelatb expression
2. match_struct sdelatb chtobi pravilno kidalo exception
3. mb current type na shared ptrx
4. ispravitb types
*/
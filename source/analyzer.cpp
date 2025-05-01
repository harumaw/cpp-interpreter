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

void Analyzer::visit(FuncDeclaration& node){
    auto returnable = node.type;
    auto name = node.declarator;
    auto type = get_type(returnable);
    auto args = node.args;
    auto block = node.body;
    std::vector<Type> type_args;
    
    scope = scope->create_new_table(scope, block);
    
    for(auto i : args){
        this->visit(*i);
        type_args.push_back(current_type);
        scope -> push_variable(i->init_declarator->declarator->name, current_type);
    }
    auto func = FuncType(type, type_args);
    scope->push_func(name->name, func);
    this->visit(*block);
    scope = scope->get_prev_table();
    current_type = func;

}

void Analyzer::visit(ParameterDeclaration& node){
    auto type = node.type;
    auto init_declarator = node.init_declarator;
    current_type = get_type(type);
    this->visit(*init_declarator);
}

void Analyzer::visit(StructDeclaration& node){
    auto id = node.name;
    auto members = node.members;
    std::unordered_map<std::string, Type> struct_vars; // add methods in structure
    for(auto member : members){
        this->visit(*member);
        auto name = member->type;
        struct_vars[name] = current_type;
    }
    auto str = StructType(struct_vars); // peredealtb
    scope -> push_struct(id, str);
    current_type = str;
}




Type Analyzer::get_type(Token token){
   if (token == TokenType::ID){
        return scope->match_struct(token.value);
    } else {
        return default_types.at(token.value);
    }
}
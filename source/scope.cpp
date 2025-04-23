#include "scope.hpp"
#include <stdexcept>

Scope::Scope(std::shared_ptr<Scope> prev_scope, std::shared_ptr<ASTNode> node)
    : prev_table(std::move(prev_scope)), node(std::move(node)) {}

std::shared_ptr<Scope> Scope::get_prev_table() const {
    return prev_table;
}

Type Scope::match_variable(const std::string& name) {
    if (variables.contains(name)) {
        return variables.at(name);
    }
    if (prev_table) {
        return prev_table->match_variable(name);
    }
    throw std::runtime_error("Variable not found: " + name);
}

StructType Scope::match_struct(const std::string& name) {
    if (structs.contains(name)) {
        return structs.at(name);
    }
    if (prev_table) {
        return prev_table->match_struct(name);
    }
    throw std::runtime_error("Struct not found: " + name);
}

FuncType Scope::match_function(std::string name, std::vector<Type> args){
    auto range = functions.equal_range(name);
    std::map<FuncType, int> functions; 
    for (auto i = range.first; i != range.second; ++i){
        functions.insert({i->second, 0}); 
    }
    for (auto function : functions){
        auto func_args = function.first.get_args();
        if (func_args.size() != args.size()){
            functions.erase(function.first);
        }
        bool match = true;
        for (size_t j = 0; j < args.size(); ++j) {
            if (typeid(args[j]) != typeid(func_args[j])) {
                match = false; 
                break;
            }
            // TODO реализовать чтоб типы были конвертируемы сейчас ищет только идеальное совпадение типов
        }

        if (match) {
            return function.first; 
        }
    }
    if (prev_table.get() == nullptr){
        throw; 
    }
    
    return prev_table->match_function(name, args);
}

void Scope::push_variable(const std::string& name, Type type) {
    variables.insert({name, type});
}

void Scope::push_struct(const std::string& name, StructType type) {
    structs.insert({name, type});
}

void Scope::push_func(const std::string& name, FuncType func) {
    functions.insert({name, func});
}

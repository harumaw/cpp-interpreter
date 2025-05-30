#include <unordered_map>
#include <iostream>
#include <set>
#include "scope.hpp"
#include "memory"

Scope::Scope(std::shared_ptr<Scope> prev_table  = nullptr) : 
    prev_table(prev_table) {}

std::shared_ptr<Scope> Scope::get_prev_table() {
    return this->prev_table;
}

std::shared_ptr<Scope> Scope::create_new_table(std::shared_ptr<Scope> prev_scope) {
    auto scope = std::make_shared<Scope>(prev_scope);
    return scope;
}   

bool Scope::contains_symbol(std::string name) {
    return symbolTable.find(name) != symbolTable.end();
}

std::shared_ptr<Symbol> Scope::match_global(std::string name) {
    if (contains_symbol(name)) {
        return symbolTable.find(name)->second;
    }
    if (prev_table == nullptr){
        throw std::runtime_error("Symbol '" + name + "' not found in scope.");
    }
    return this->prev_table->match_global(name); // возвращаем из старшей области видимости
}

std::shared_ptr<Symbol> Scope::match_local(std::string name) {
    if (contains_symbol(name)) {
        return symbolTable.find(name)->second;
    }
    throw std::runtime_error("Symbol '" + name + "' not found in local scope.");
}

std::vector<std::shared_ptr<Symbol>> Scope::match_range(std::string name) {
    std::vector<std::shared_ptr<Symbol>> result;
    auto range = symbolTable.equal_range(name); // Получаем диапазон элементов с одинаковым ключом
    for (auto it = range.first; it != range.second; ++it) {
        result.push_back(it->second);
    }
    if (result.empty() && prev_table != nullptr) {
        return this->prev_table->match_range(name); // Ищем в старшей области видимости
    }
    if (result.empty()) {
        throw std::runtime_error("Symbol '" + name + "' not found in scope.");
    }
    return result;
}

void Scope::push_symbol(std::string name, std::shared_ptr<Symbol> symbol) {
    if (dynamic_cast<FuncSymbol*>(symbol.get()) && contains_symbol(name)) {
        
    }
    if (contains_symbol(name)) {
        throw std::runtime_error("Symbol '" + name + "' already exists in scope. in scope");
    }
    symbolTable.insert({name, symbol});
}
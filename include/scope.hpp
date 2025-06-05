#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <any>
#include <vector>
#include "type.hpp"
#include "ast.hpp"
#include "symbol.hpp"

struct Scope {
public:
    ~Scope() = default;
    Scope(std::shared_ptr<Scope> get_prev_table);
    
    std::shared_ptr<Scope> get_prev_table();
    std::shared_ptr<Scope> create_new_table(std::shared_ptr<Scope>);
    
    std::shared_ptr<Symbol> match_global(std::string);
    std::shared_ptr<Symbol> match_local(std::string);
    std::vector<std::shared_ptr<Symbol>> match_range(std::string);
    bool contains_symbol(std::string);
    std::multimap<std::string, std::shared_ptr<Symbol>> get_symbols() {
        return symbolTable;
    }
    bool contains_symbol_recursive(const std::string& name);


    
    void push_symbol(std::string, std::shared_ptr<Symbol>);
private:
    std::shared_ptr<Scope>prev_table;
    std::multimap<std::string, std::shared_ptr<Symbol>> symbolTable;
};
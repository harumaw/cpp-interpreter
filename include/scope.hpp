
#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <memory>
#include <stdexcept>
#include "type.hpp"
#include "ast.hpp"

class Scope : public std::enable_shared_from_this<Scope> {
public:
    Scope(std::shared_ptr<Scope> prev_scope, std::shared_ptr<ASTNode> node);
    std::shared_ptr<Scope> create_new_table(std::shared_ptr<Scope> prev_scope,
                                            std::shared_ptr<ASTNode> node);
    std::shared_ptr<Scope> get_prev_table() const;

    std::shared_ptr<Type> match_variable(const std::string& name);
    std::shared_ptr<Type> match_struct(const std::string& name);
    std::shared_ptr<FuncType> match_function(const std::string& name,
                                             const std::vector<std::shared_ptr<Type>>& args);

    void push_variable(const std::string& name, std::shared_ptr<Type> type);
    void push_struct(const std::string& name, std::shared_ptr<Type> type);
    void push_func(const std::string& name, std::shared_ptr<FuncType> func);
    bool has_variable(const std::string& name) const;

private:
    std::shared_ptr<Scope> prev_table;
    std::shared_ptr<ASTNode> node;

    std::unordered_map<std::string, std::shared_ptr<Type>> variables;
    std::unordered_map<std::string, std::shared_ptr<Type>> structs;
    std::multimap<std::string, std::shared_ptr<FuncType>> functions;
};

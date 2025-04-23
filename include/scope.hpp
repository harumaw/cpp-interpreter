#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <any>
#include <vector>
#include <memory>
#include "type.hpp"
#include "ast.hpp"

class Scope : public std::enable_shared_from_this<Scope> {
public:
    Scope(std::shared_ptr<Scope> prev_scope, std::shared_ptr<ASTNode> node);

    std::shared_ptr<Scope> get_prev_table() const;

    Type match_variable(const std::string& name);
    StructType match_struct(const std::string& name);
    FuncType match_function(std::string name, std::vector<Type> args);

    void push_variable(const std::string& name, Type type);
    void push_struct(const std::string& name, StructType type);
    void push_func(const std::string& name, FuncType func);

private:
    std::shared_ptr<Scope> prev_table;
    std::shared_ptr<ASTNode> node;

    std::unordered_map<std::string, Type> variables;
    std::unordered_map<std::string, StructType> structs;
    std::multimap<std::string, FuncType> functions;
};

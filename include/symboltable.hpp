#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <stdexcept>
#include "declaration.hpp"
#include "type.hpp"

struct Value {
    enum class Type { Integer, Float, Char, Bool, Void, Struct } type;
    int64_t    i{};
    float     f{};
    char16_t   c{};
    bool       b{};
    std::unordered_map<std::string, Value> members;

    static Value makeInt(int64_t v){
        Value x;
        x.type = Type::Integer;
        x.i = v;
        return x;
    }

    static Value makeFloat(float v){
        Value x;
        x.type = Type::Float;
        x.f = v;
        return x;
    }

    static Value makeChar(char16_t v){
        Value x;
        x.type = Type::Char;
        x.c = v;
        return x;
    }

    static Value makeBool(bool v){
        Value x;
        x.type = Type::Bool;
        x.b = v;
        return x;
    }   

    static Value makeStruct(std::unordered_map<std::string, Value> members){
        Value x;
        x.type = Type::Struct;
        x.members = members;
        return x;
    }   
    
    static Value makeVoid(){
        Value x;
        x.type = Type::Void;
        return x;
    }

};

class SymbolTable {
public:
    SymbolTable(std::shared_ptr<SymbolTable> prev_table, std::shared_ptr<ASTNode> node) {}

    std::shared_ptr<SymbolTable> get_prev_table();    
    std::shared_ptr<SymbolTable> create_new_table(std::shared_ptr<SymbolTable> prev_table, std::shared_ptr<ASTNode> node);

    void push_variable(const std::string& name, const Value& value);
    Value match_variable(const std::string& name) const;

    void push_function(const std::string& name, const std::function<Value(const std::vector<Value>&)>& fn);
    std::function<Value(const std::vector<Value>&)> match_function(const std::string& name) const;

    void push_struct(const std::string& name, const std::unordered_map<std::string, Value::Type>& schema);
    std::unordered_map<std::string, Value::Type>& match_struct(const std::string& name) const;

    void push_namespace(const std::string& name, std::shared_ptr<SymbolTable> table);
    std::shared_ptr<SymbolTable> match_namespace(const std::string& name) const;





private:
    std::shared_ptr<SymbolTable> prev_table;
    std::shared_ptr<ASTNode> node;

    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, std::function<Value(const std::vector<Value>&)>> functions;
    std::unordered_map<std::string, std::unordered_map<std::string, Value::Type>> structs;
    std::unordered_map<std::string, std::shared_ptr<SymbolTable>> namespaces;
};
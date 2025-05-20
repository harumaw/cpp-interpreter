#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#include "ast.hpp"     
#include "declaration.hpp"       
#include "semantic_exception.hpp"


struct Value {
    enum class Type { Integer, Float, Char, Bool, Void } type;
    int64_t    i;
    double     f;
    char16_t   c;
    bool       b;

    static Value makeInt(int64_t v)   { Value x {Type::Integer, v, 0, 0, false}; return x; }
    static Value makeFloat(double v)  { Value x {Type::Float,    0, v, 0, false}; return x; }
    static Value makeChar(char16_t v) { Value x {Type::Char,     0, 0, v,  false}; return x; }
    static Value makeBool(bool v)     { Value x {Type::Bool,     0, 0, 0,  v   }; return x; }
    static Value makeVoid()           { Value x {Type::Void,     0, 0, 0,  false}; return x; }
};

class SymbolTable {
public:
    SymbolTable(std::shared_ptr<SymbolTable> prev_table = nullptr) : prev_table(prev_table) {}

    std::shared_ptr<SymbolTable> get_prev_table();
    
    std::shared_ptr<SymbolTable> create_new_table();

    void push_variable(const std::string& name, const Value& value);
    std::shared_ptr<Value> match_variable(const std::string& name);


    void push_function(const std::string& name, std::function<Value(const std::vector<Value>&)> fn);
    std::function<Value(const std::vector<Value>&)> match_function(const std::string& name);

    void push_struct(const std::string& name, std::vector<std::pair<std::string, Value::Type>> members);
    const std::vector<std::pair<std::string, Value::Type>>& match_struct();

    void push_namespace(const std::string& name, std::shared_ptr<SymbolTable> ns);
    std::shared_ptr<SymbolTable> match_namespace(const std::string& name);


private:
    std::shared_ptr<SymbolTable> prev_table;

    std::unordered_map<std::string, std::shared_ptr<Value>> variables;
    std::unordered_map<std::string, std::function<Value(const std::vector<Value>&)>> functions;
    std::unordered_map<std::string, std::vector<std::pair<std::string, Value::Type>>> structs;
    std::unordered_map<std::string, std::shared_ptr<SymbolTable>> namespaces;

};


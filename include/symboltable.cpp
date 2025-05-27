#include "symboltable.hpp"


SymbolTable::SymbolTable(std::shared_ptr<SymbolTable> prev_table,
                         std::shared_ptr<ASTNode> node)
  : prev_table(std::move(prev_table)), node(std::move(node))
{}

std::shared_ptr<SymbolTable>
SymbolTable::create_new_table(std::shared_ptr<SymbolTable> prev_table,
                              std::shared_ptr<ASTNode>     node)
{
    return std::make_shared<SymbolTable>(std::move(prev_table), std::move(node));
}

std::shared_ptr<SymbolTable> SymbolTable::get_prev_table() {
    return prev_table;
}

void SymbolTable::push_variable(const std::string& name, const Value& value) {
    if (variables.count(name))
        throw std::runtime_error("variable already defined: " + name);
    variables[name] = value;
}

Value SymbolTable::match_variable(const std::string& name) const {
    auto it = variables.find(name);
    if (it != variables.end())
        return it->second;
    if (prev_table)
        return prev_table->match_variable(name);
    throw std::runtime_error("undefined variable: " + name);
}

void SymbolTable::push_function(const std::string& name,
    const std::function<Value(const std::vector<Value>&)>& fn)
{
    if (functions.count(name))
        throw std::runtime_error("function already defined: " + name);
    functions[name] = fn;
}

std::function<Value(const std::vector<Value>&)>
SymbolTable::match_function(const std::string& name) const
{
    auto it = functions.find(name);
    if (it != functions.end())
        return it->second;
    if (prev_table)
        return prev_table->match_function(name);
    throw std::runtime_error("undefined function: " + name);
}

void SymbolTable::push_struct(const std::string& name,
    const std::unordered_map<std::string, Value::Type>& schema)
{
    if (structs.count(name))
        throw std::runtime_error("struct already defined: " + name);
    structs[name] = schema;
}

std::unordered_map<std::string, Value::Type>&
SymbolTable::match_struct(const std::string& name) const
{
    auto it = structs.find(name);
    if (it != structs.end())
        return const_cast<std::unordered_map<std::string, Value::Type>&>(it->second);
    if (prev_table)
        return prev_table->match_struct(name);
    throw std::runtime_error("undefined struct: " + name);
}

void SymbolTable::push_namespace(const std::string& name,
                                 std::shared_ptr<SymbolTable> table)
{
    if (namespaces.count(name))
        throw std::runtime_error("namespace already defined: " + name);
    namespaces[name] = std::move(table);
}

std::shared_ptr<SymbolTable>
SymbolTable::match_namespace(const std::string& name) const
{
    auto it = namespaces.find(name);
    if (it != namespaces.end())
        return it->second;
    if (prev_table)
        return prev_table->match_namespace(name);
    throw std::runtime_error("undefined namespace: " + name);
}
*/
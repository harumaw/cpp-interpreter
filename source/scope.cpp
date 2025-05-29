#include "scope.hpp"

Scope::Scope(std::shared_ptr<Scope> prev_scope, std::shared_ptr<ASTNode> node)
    : prev_table(std::move(prev_scope)), node(std::move(node)) {}

std::shared_ptr<Scope> Scope::get_prev_table() const {
    return prev_table;
}

std::shared_ptr<Scope> Scope::create_new_table(std::shared_ptr<Scope> prev_scope,
                                              std::shared_ptr<ASTNode> node) {
    return std::make_shared<Scope>(std::move(prev_scope), std::move(node));
}

std::shared_ptr<Type> Scope::match_variable(const std::string& name) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    if (prev_table) {
        return prev_table->match_variable(name);
    }

    for (auto& [ns_name, ns_scope] : namespaces) {
        try {
            return ns_scope->match_variable(name);
        } catch (...) {
        }
    }


    throw std::runtime_error("Variable not found: " + name);
}




std::shared_ptr<Type> Scope::match_struct(const std::string& name) {

    if (auto it = structs.find(name); it != structs.end())
        return it->second;

    // 2) родительский
    if (prev_table) {
        try {
            return prev_table->match_struct(name);
        } catch (...) {}
    }

    // 3) пространства имён
    for (auto& [ns_name, ns_scope] : namespaces) {
        try {
            return ns_scope->match_struct(name);
        } catch (...) {}
    }

    throw std::runtime_error("Struct not found: " + name);
}

std::shared_ptr<FuncType> Scope::match_function(const std::string& name,
                                                 const std::vector<std::shared_ptr<Type>>& args) {
    auto range = functions.equal_range(name);
    for (auto it = range.first; it != range.second; ++it) {
        auto cand = it->second;
        auto cand_args = cand->get_args();
        if (cand_args.size() != args.size()) continue;
        bool ok = true;
        for (size_t i = 0; i < args.size(); ++i) {
            if (!args[i]->equals(cand_args[i])) {
                ok = false;
                break;
            }
        }
        if (ok) return cand;
    }
    if (prev_table) {
        return prev_table->match_function(name, args);
    }
    throw std::runtime_error("Function not found: " + name);
}

std::shared_ptr<Scope> Scope::match_namespace(const std::string& name) {
    auto it = namespaces.find(name);
    if (it != namespaces.end()) {
        return it->second;
    }
    if (prev_table) {
        return prev_table->match_namespace(name);
    }
    throw std::runtime_error("Namespace not found: " + name);
}

void Scope::push_namespace(const std::string& name, std::shared_ptr<Scope> scope) {
    namespaces.emplace(name, std::move(scope));
}
void Scope::push_variable(const std::string& name, std::shared_ptr<Type> type) {
    variables.emplace(name, std::move(type));
}

void Scope::push_struct(const std::string& name, std::shared_ptr<Type> type) {
    structs.emplace(name, std::move(type));
}

void Scope::push_func(const std::string& name, std::shared_ptr<FuncType> func) {
    functions.emplace(name, std::move(func));
}
bool Scope::has_variable_current(const std::string& name) const {
    return variables.find(name) != variables.end();
}

bool Scope::has_variable(const std::string& name) const {
        // 1) сначала в этом скоупе
    if (variables.find(name) != variables.end())
        return true;

    // 2) затем в родительском скоупе
    if (prev_table && prev_table->has_variable(name))
        return true;

    // 3) затем в каждом вложенном namespace
    for (auto& [ns_name, ns_scope] : namespaces) {
        if (ns_scope->has_variable(name))
            return true;
    }

    // не найдено
    return false;
}

bool Scope::has_struct(const std::string& name) const {
     // 1) сначала в этом скоупе
    if (structs.find(name) != structs.end())
        return true;
    // 2) затем в родительском
    if (prev_table && prev_table->has_struct(name))
        return true;
    // 3) затем во всех вложенных namespace
    for (auto& [ns_name, ns_scope] : namespaces) {
        if (ns_scope->has_struct(name))
            return true;
    }
    // не нашли ни в одном месте
    return false;
}

bool Scope::has_namespace(const std::string& name) const {
    if (namespaces.count(name)) return true;
    if (prev_table) return prev_table->has_namespace(name);
    return false;
}


bool Scope::has_function(const std::string& name,
                         const std::vector<std::shared_ptr<Type>>& args) const {
    auto range = functions.equal_range(name);
    for (auto it = range.first; it != range.second; ++it) {
        auto cand = it->second;
        auto params = cand->get_args();
        if (params.size() != args.size()) continue;
        bool ok = true;
        for (size_t i = 0; i < args.size(); ++i) {
            if (!args[i]->equals(params[i])) { ok = false; break; }
        }
        if (ok) return true;
    }
    if (prev_table) return prev_table->has_function(name, args);
    return false;
}
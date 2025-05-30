#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "type.hpp"
#include "scope.hpp"


struct Scope;

struct Symbol {
    std::shared_ptr<Type> type;
    Symbol(std::shared_ptr<Type> t) : type(std::move(t)) {}
    virtual ~Symbol() = default;
};

struct VarSymbol : Symbol {
    VarSymbol(std::shared_ptr<Type> t)
      : Symbol(std::move(t)) {}

    VarSymbol(std::shared_ptr<Type> t, std::any v) : Symbol(std::move(t)), value(std::move(v)) {}

    std::any value;
};

struct FuncSymbol : Symbol {
    std::vector<std::shared_ptr<Type>> params;
    bool isConstMethod = false;

    FuncSymbol(std::shared_ptr<FuncType> ft,
               std::vector<std::shared_ptr<Type>> p = {},
               bool constMethod = false)
      : Symbol(std::move(ft))
      , params(std::move(p))
      , isConstMethod(constMethod)
    {}
};


struct RecordSymbol : Symbol {
    std::unordered_map<std::string, std::shared_ptr<Symbol>> members;

    RecordSymbol(std::shared_ptr<RecordType> rt,
                 std::unordered_map<std::string, std::shared_ptr<Symbol>> m)
      : Symbol(std::move(rt))
      , members(std::move(m))
    {}
};


struct StructSymbol : RecordSymbol {
    StructSymbol(std::shared_ptr<RecordType> rt,
                 std::unordered_map<std::string, std::shared_ptr<Symbol>> m)
      : RecordSymbol(std::move(rt), std::move(m))
    {}
};


struct NamespaceSymbol : RecordSymbol {
    std::shared_ptr<Scope> scope;

    NamespaceSymbol(std::shared_ptr<Scope> s)
      : RecordSymbol(nullptr, {})
      , scope(std::move(s))
    {}
};

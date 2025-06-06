#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <any>

// ——— Форвард-объявления: эти классы определяются в type.hpp/scope.hpp, 
//    но нам нужно знание их имён уже здесь.
struct Type;
struct FuncType;
struct RecordType;
struct StructType;
struct Scope;
struct FuncDeclaration;


struct Symbol {
    std::shared_ptr<Type> type;
    Symbol(std::shared_ptr<Type> t) : type(std::move(t)) {}
    virtual ~Symbol() = default;
};

struct VarSymbol : Symbol {
    VarSymbol(std::shared_ptr<Type> t)
      : Symbol(std::move(t)) {}

    VarSymbol(std::shared_ptr<Type> t, std::any v)
      : Symbol(std::move(t)), value(std::move(v)) {}

    std::any value;
};

struct FuncSymbol : Symbol {
    std::vector<std::shared_ptr<Type>> params;
    bool isConstMethod = false;
    FuncDeclaration* declaration;
    FuncSymbol(std::shared_ptr<FuncType> ft,
               std::vector<std::shared_ptr<Type>> p = {},
               bool constMethod = false)
      : Symbol(std::static_pointer_cast<Type>(std::move(ft)))
      , params(std::move(p))
      , isConstMethod(constMethod),
        declaration(nullptr)
    {}

    // Пустой конструктор (на всякий случай)
    FuncSymbol() : Symbol(nullptr), isConstMethod(false), declaration(nullptr) {}
};

struct RecordSymbol : Symbol {
    std::unordered_map<std::string, std::shared_ptr<Symbol>> members;

    RecordSymbol(std::shared_ptr<RecordType> rt,
                 std::unordered_map<std::string, std::shared_ptr<Symbol>> m)
      : Symbol(std::static_pointer_cast<Type>(std::move(rt)))
      , members(std::move(m))
    {}

    RecordSymbol() : Symbol(nullptr) {}
};
struct StructSymbol : RecordSymbol {
    StructSymbol(std::shared_ptr<StructType> st,
                 std::unordered_map<std::string, std::shared_ptr<Symbol>> m)
      : RecordSymbol(std::static_pointer_cast<RecordType>(std::move(st)), std::move(m))
    {}

    StructSymbol() : RecordSymbol() {}
};


struct NamespaceSymbol : RecordSymbol {
    std::shared_ptr<Scope> scope;

    NamespaceSymbol(std::shared_ptr<Scope> s)
      : RecordSymbol(nullptr, {})
      , scope(std::move(s))
    {}

    NamespaceSymbol() : RecordSymbol(), scope(nullptr) {}
};

struct ArrayElementSymbol : VarSymbol {
    std::shared_ptr<VarSymbol> parentArray;
    int index;
    ArrayElementSymbol(std::shared_ptr<Type> elemType,
                       const std::any &elemValue,
                       std::shared_ptr<VarSymbol> parent,
                       int idx)
      : VarSymbol(elemType, elemValue),
        parentArray(std::move(parent)),
        index(idx)
    {}
};

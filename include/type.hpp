#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <any>
#include <memory>
#include <iostream>
#include "expression.hpp"
#include "symbol.hpp"

/*
Type
├─ Fundamental
│   ├─ VoidType
│   ├─ NullPtrType
│   ├─ Arithmetic
│   │   ├─ Integral
│   │   │   ├─ BoolType
│   │   │   ├─ CharType
│   │   │   └─ IntegerType
│   │   └─ FloatType
│   └─ StringType
└─ Composite
    ├─ FuncType
    ├─ Record
    │   └─ StructType
    ├─ PointerType
    ├─ RefType
    │   ├─ LValueType
    │   └─ RValueType
    └─ ArrayType
*/

struct Type {
    virtual ~Type();
    virtual bool equals(const std::shared_ptr<Type>& other) const = 0;
    virtual void print();
};

// ---------------------------
// Фундаментальные типы
// ---------------------------
struct Fundamental : Type {};

struct VoidType : Fundamental {
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
};

struct NullPtrType : Fundamental {
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
};

struct Arithmetic : Fundamental {
    explicit Arithmetic(std::any value);
    virtual ~Arithmetic();                      
    std::any get_any_value() const;
    void print() override;
protected:
    std::any value;
};

struct Integral : Arithmetic {
    explicit Integral(std::any value);
    virtual ~Integral();                       
    void print() override;
};

struct BoolType : Integral {
    explicit BoolType(std::any value = false);
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
};

struct CharType : Integral {
    explicit CharType(std::any value = u'\0');
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
};

struct IntegerType : Integral {
    explicit IntegerType(std::any value = int8_t(0));
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
    std::any value;
};

struct FloatType : Arithmetic {
    explicit FloatType(std::any value = 0.0);
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
};

struct StringType : Fundamental {
    explicit StringType(std::string value = "");
    std::string get_value() const;
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
private:
    std::string value;
};

// ---------------------------
// Составные типы
// ---------------------------
struct Composite : Type {}; 

struct FuncType : Composite {
    explicit FuncType(std::shared_ptr<Type> return_type,
                      std::vector<std::shared_ptr<Type>> args,
                      bool is_method_c = false);
    std::shared_ptr<Type> get_returnable_type() const;
    std::vector<std::shared_ptr<Type>> get_args() const;
    bool is_method_const() const;
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
private:
    std::shared_ptr<Type> returnable_type;
    std::vector<std::shared_ptr<Type>> args;
    bool is_method_c;
};

struct RecordType : Composite {};

struct StructType : RecordType {
    explicit StructType(const std::unordered_map<std::string, std::shared_ptr<Type>>& members,
                        const std::unordered_map<std::string, std::shared_ptr<FuncType>>& methods);
    std::unordered_map<std::string, std::shared_ptr<Type>> get_members() const;
    std::unordered_map<std::string, std::shared_ptr<FuncType>> get_methods() const;
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
private:
    std::unordered_map<std::string, std::shared_ptr<Type>> members;
    std::unordered_map<std::string, std::shared_ptr<FuncType>> methods;
};

struct PointerType : Composite {
    explicit PointerType(std::shared_ptr<Type> base = nullptr);
    std::shared_ptr<Type> get_base() const;
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
private:
    std::shared_ptr<Type> base;
};

struct RefType : Composite {};

struct LValueType : RefType {
    explicit LValueType(std::shared_ptr<Type> ref_to);
    std::shared_ptr<Type> get_referenced_type() const;
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
private:
    std::shared_ptr<Type> ref_to;
};

struct RValueType : RefType {
    explicit RValueType(std::shared_ptr<Type> ref_to);
    std::shared_ptr<Type> get_referenced_type() const;
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
private:
    std::shared_ptr<Type> ref_to;
};

struct ArrayType : Composite {
    explicit ArrayType(std::shared_ptr<Type> base, std::shared_ptr<Expression> size);
    std::shared_ptr<Type> get_base_type() const;
    expression get_size() const; // rework in int 
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
private:
    std::shared_ptr<Type> base;
    std::shared_ptr<Expression> size;
};

struct ConstType : Type {
    explicit ConstType(std::shared_ptr<Type> base);
    std::shared_ptr<Type> get_base() const;
    bool equals(const std::shared_ptr<Type>& other) const override;
    void print() override;
private:
    std::shared_ptr<Type> base;
};

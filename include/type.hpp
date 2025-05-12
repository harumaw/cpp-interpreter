#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <any>
#include <memory>

#include "expression.hpp"

struct Type {
    virtual ~Type() = default;
    virtual bool equals(const std::shared_ptr<Type>& other) const = 0;
};

// ---------------------------
// Фундаментальные типы
// ---------------------------
struct Fundamental : Type {};

struct VoidType : Fundamental {
    bool equals(const std::shared_ptr<Type>& other) const override;
};

struct NullPtrType : Fundamental {
    bool equals(const std::shared_ptr<Type>& other) const override;
};

struct Arithmetic : Fundamental {
    explicit Arithmetic(std::any value);
    std::any get_any_value() const;

protected:
    std::any value;
};

struct Integral : Arithmetic {
    explicit Integral(std::any value);
};

struct BoolType : Integral {
    explicit BoolType(std::any value = false);
    bool equals(const std::shared_ptr<Type>& other) const override;
};

struct CharType : Integral {
    explicit CharType(std::any value = u'\0');
    bool equals(const std::shared_ptr<Type>& other) const override;
};

struct IntegerType : Integral {
    explicit IntegerType(std::any value = int8_t(0));
    bool equals(const std::shared_ptr<Type>& other) const override;
};

struct FloatType : Arithmetic {
    explicit FloatType(std::any value = 0.0);
    bool equals(const std::shared_ptr<Type>& other) const override;
};

struct StringType : Fundamental {
    explicit StringType(std::string value = "");
    std::string get_value() const;
    bool equals(const std::shared_ptr<Type>& other) const override;

private:
    std::string value;
};

// ---------------------------
// Составные типы
// ---------------------------
struct Composite : Type {};

struct FuncType : Composite {
    FuncType(std::shared_ptr<Type> return_type, std::vector<std::shared_ptr<Type>> args);

    std::shared_ptr<Type> get_returnable_type() const;
    std::vector<std::shared_ptr<Type>> get_args() const;
    bool equals(const std::shared_ptr<Type>& other) const override;

private:
    std::shared_ptr<Type> returnable_type;
    std::vector<std::shared_ptr<Type>> args;
};

struct Record : Composite {};

struct StructType : Record {
    explicit StructType(const std::unordered_map<std::string, std::shared_ptr<Type>>& members);
    std::unordered_map<std::string, std::shared_ptr<Type>> get_members() const;
    bool equals(const std::shared_ptr<Type>& other) const override;

private:
    std::unordered_map<std::string, std::shared_ptr<Type>> members;
};

struct PointerType : Composite {
    explicit PointerType(std::shared_ptr<Type> base = nullptr);
    std::shared_ptr<Type> get_base() const;
    bool equals(const std::shared_ptr<Type>& other) const override;

private:
    std::shared_ptr<Type> base;
};

struct RefType : Composite {};

struct LValueType : RefType {
    explicit LValueType(std::shared_ptr<Type> ref_to);
    std::shared_ptr<Type> get_referenced_type() const;
    bool equals(const std::shared_ptr<Type>& other) const override;

private:
    std::shared_ptr<Type> ref_to;
};

struct RValueType : RefType {
    explicit RValueType(std::shared_ptr<Type> ref_to);
    std::shared_ptr<Type> get_referenced_type() const;
    bool equals(const std::shared_ptr<Type>& other) const override;

private:
    std::shared_ptr<Type> ref_to;
};

struct ArrayType : Composite {
    ArrayType(std::shared_ptr<Type> base, expression size);
    std::shared_ptr<Type> get_base_type() const;
    expression get_size() const;
    bool equals(const std::shared_ptr<Type>& other) const override;

private:
    std::shared_ptr<Type> base;
    expression size;
};

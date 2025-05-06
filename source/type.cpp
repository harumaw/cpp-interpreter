#include "type.hpp"

// ---------------------------
// Фундаментальные типы
// ---------------------------
Arithmetic::Arithmetic(std::any value) : value(std::move(value)) {}

std::any Arithmetic::get_any_value() const {
    return value;
}

Integral::Integral(std::any value) : Arithmetic(std::move(value)) {}

BoolType::BoolType(std::any value) : Integral(std::move(value)) {
    this->value = std::any_cast<bool>(get_any_value());
}

CharType::CharType(std::any value) : Integral(std::move(value)) {
    this->value = std::any_cast<char16_t>(get_any_value());
}

IntegerType::IntegerType(std::any value) : Integral(std::move(value)) {
    this->value = std::any_cast<int8_t>(get_any_value());
}

FloatType::FloatType(std::any value) : Arithmetic(std::move(value)) {
    this->value = std::any_cast<double>(get_any_value());
}

FuncType::FuncType(Type return_type, std::vector<Type> args)
    : returnable_type(std::move(return_type)), args(std::move(args)) {}

Type FuncType::get_returnable_type() const {
    return returnable_type;
}

/*std::vector<std::shared_ptr<Type>> FuncType::get_args() const {
    return args;
}*/

StructType::StructType(const std::unordered_map<std::string, Type>& members)
    : members(members) {}

std::unordered_map<std::string, Type> StructType::get_members() const {
    return members;
}

PointerType::PointerType(std::shared_ptr<Type> base) : base(std::move(base)) {}

std::shared_ptr<Type> PointerType::get_base() const {
    return base;
}

LValueType::LValueType(std::shared_ptr<Type> ref_to) : ref_to(std::move(ref_to)) {}

std::shared_ptr<Type> LValueType::get_referenced_type() const {
    return ref_to;
}

RValueType::RValueType(std::shared_ptr<Type> ref_to) : ref_to(std::move(ref_to)) {}

std::shared_ptr<Type> RValueType::get_referenced_type() const {
    return ref_to;
}

ArrayType::ArrayType(Type base, expression size) : base(std::move(base)), size(size) {}

Type ArrayType::get_base_type() const {
    return base; // Возвращаем объект Type
}

expression ArrayType::get_size() const {
    return size;
}
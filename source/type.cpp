// type.cpp
#include "type.hpp"
#include <typeinfo>

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

StringType::StringType(std::string value) : value(std::move(value)) {}

std::string StringType::get_value() const {
    return value;
}

// ---------------------------
// equals() реализации для всех типов
// ---------------------------
bool VoidType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<VoidType*>(other.get()) != nullptr;
}

bool NullPtrType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<NullPtrType*>(other.get()) != nullptr;
}

bool BoolType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<BoolType*>(other.get()) != nullptr;
}

bool CharType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<CharType*>(other.get()) != nullptr;
}

bool IntegerType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<IntegerType*>(other.get()) != nullptr;
}

bool FloatType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<FloatType*>(other.get()) != nullptr;
}

bool StringType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<StringType*>(other.get()) != nullptr;
}

FuncType::FuncType(std::shared_ptr<Type> return_type, std::vector<std::shared_ptr<Type>> args)
    : returnable_type(std::move(return_type)), args(std::move(args)) {}

std::shared_ptr<Type> FuncType::get_returnable_type() const {
    return returnable_type;
}

std::vector<std::shared_ptr<Type>> FuncType::get_args() const {
    return args;
}

bool FuncType::equals(const std::shared_ptr<Type>& other) const {
    auto o = dynamic_cast<FuncType*>(other.get());
    if (!o || !returnable_type->equals(o->returnable_type) || args.size() != o->args.size()) return false;
    for (size_t i = 0; i < args.size(); ++i) {
        if (!args[i]->equals(o->args[i])) return false;
    }
    return true;
}

StructType::StructType(const std::unordered_map<std::string, std::shared_ptr<Type>>& members)
    : members(members) {}

std::unordered_map<std::string, std::shared_ptr<Type>> StructType::get_members() const {
    return members;
}

bool StructType::equals(const std::shared_ptr<Type>& other) const {
    auto o = dynamic_cast<StructType*>(other.get());
    if (!o || o->members.size() != members.size()) return false;
    for (const auto& [key, val] : members) {
        auto it = o->members.find(key);
        if (it == o->members.end() || !val->equals(it->second)) return false;
    }
    return true;
}

PointerType::PointerType(std::shared_ptr<Type> base) : base(std::move(base)) {}

std::shared_ptr<Type> PointerType::get_base() const {
    return base;
}

bool PointerType::equals(const std::shared_ptr<Type>& other) const {
    auto o = dynamic_cast<PointerType*>(other.get());
    return o && base->equals(o->base);
}

LValueType::LValueType(std::shared_ptr<Type> ref_to) : ref_to(std::move(ref_to)) {}

std::shared_ptr<Type> LValueType::get_referenced_type() const {
    return ref_to;
}

bool LValueType::equals(const std::shared_ptr<Type>& other) const {
    auto o = dynamic_cast<LValueType*>(other.get());
    return o && ref_to->equals(o->ref_to);
}

RValueType::RValueType(std::shared_ptr<Type> ref_to) : ref_to(std::move(ref_to)) {}

std::shared_ptr<Type> RValueType::get_referenced_type() const {
    return ref_to;
}

bool RValueType::equals(const std::shared_ptr<Type>& other) const {
    auto o = dynamic_cast<RValueType*>(other.get());
    return o && ref_to->equals(o->ref_to);
}

ArrayType::ArrayType(std::shared_ptr<Type> base, expression size) : base(std::move(base)), size(size) {}

std::shared_ptr<Type> ArrayType::get_base_type() const {
    return base;
}

expression ArrayType::get_size() const {
    return size;
}

bool ArrayType::equals(const std::shared_ptr<Type>& other) const {
    auto o = dynamic_cast<ArrayType*>(other.get());
 
    return o && base->equals(o->base);
}



ConstType::ConstType(std::shared_ptr<Type> base) : base(base) {}
std::shared_ptr<Type> ConstType::get_base(){
    return base;
}

bool ConstType::equals(const std::shared_ptr<Type>& other) const {
    auto p = std::dynamic_pointer_cast<ConstType>(other);
    if (!p) return false;
    return base->equals(p->base);
}
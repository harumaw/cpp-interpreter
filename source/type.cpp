#include "type.hpp"
#include <iostream>
#include <any>

// «Закрываем» v-таблицы для базовых абстрактных классов
Type::~Type() = default;
Arithmetic::~Arithmetic() = default;
Integral::~Integral()   = default;

// Печать по умолчанию ничего не делает
void Type::print() { }

// ---------------------------
// Фундаментальные типы
// ---------------------------

bool VoidType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<VoidType*>(other.get()) != nullptr;
}
void VoidType::print() {
    std::cout << "void";
}

bool NullPtrType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<NullPtrType*>(other.get()) != nullptr;
}
void NullPtrType::print() {
    std::cout << "nullptr";
}

// ---------------------------
// Arithmetic, Integral
// ---------------------------

Arithmetic::Arithmetic(std::any value) : value(std::move(value)) {}
std::any Arithmetic::get_any_value() const {
    return value;
}
void Arithmetic::print() {
    std::cout << "Arithmetic";
}

Integral::Integral(std::any value) : Arithmetic(std::move(value)) {}
void Integral::print() {
    std::cout << "Integral";
}

// ---------------------------
// BoolType, CharType, IntegerType, FloatType
// ---------------------------

BoolType::BoolType(std::any value) : Integral(std::move(value)) {
    this->value = std::any_cast<bool>(get_any_value());
}
bool BoolType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<BoolType*>(other.get()) != nullptr;
}
void BoolType::print() {
    std::cout << "bool";
}

CharType::CharType(std::any value) : Integral(std::move(value)) {
    this->value = std::any_cast<char16_t>(get_any_value());
}
bool CharType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<CharType*>(other.get()) != nullptr;
}
void CharType::print() {
    std::cout << "char";
}

IntegerType::IntegerType(std::any value) : Integral(std::move(value)) {
    this->value = std::any_cast<int8_t>(get_any_value());
}
bool IntegerType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<IntegerType*>(other.get()) != nullptr;
}
void IntegerType::print() {
    std::cout << "int";
}

FloatType::FloatType(std::any value) : Arithmetic(std::move(value)) {
    this->value = std::any_cast<double>(get_any_value());
}
bool FloatType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<FloatType*>(other.get()) != nullptr;
}
void FloatType::print() {
    std::cout << "float";
}

StringType::StringType(std::string value) : value(std::move(value)) {}
std::string StringType::get_value() const {
    return value;
}
bool StringType::equals(const std::shared_ptr<Type>& other) const {
    return dynamic_cast<StringType*>(other.get()) != nullptr;
}
void StringType::print() {
    std::cout << "string";
}

// ---------------------------
// FuncType
// ---------------------------

FuncType::FuncType(std::shared_ptr<Type> return_type,
                   std::vector<std::shared_ptr<Type>> args,
                   bool is_method_c)
    : returnable_type(std::move(return_type))
    , args(std::move(args))
    , is_method_c(is_method_c)
{}

std::shared_ptr<Type> FuncType::get_returnable_type() const {
    return returnable_type;
}

std::vector<std::shared_ptr<Type>> FuncType::get_args() const {
    return args;
}

bool FuncType::is_method_const() const {
    return is_method_c;
}

bool FuncType::equals(const std::shared_ptr<Type>& other) const {
    if (auto o = dynamic_cast<FuncType*>(other.get())) {
        if (!returnable_type->equals(o->returnable_type) ||
            args.size() != o->args.size() ||
            is_method_c != o->is_method_c)
            return false;
        for (size_t i = 0; i < args.size(); ++i) {
            if (!args[i]->equals(o->args[i])) return false;
        }
        return true;
    }
    return false;
}

void FuncType::print() {
    std::cout << "FuncType(";
    returnable_type->print();
    std::cout << ", args: [";
    for (auto& a : args) {
        a->print();
        std::cout << ", ";
    }
    std::cout << "], is_method_c: " << (is_method_c ? "true" : "false") << ")";
}

// ---------------------------
// StructType
// ---------------------------

StructType::StructType(
    const std::unordered_map<std::string, std::shared_ptr<Type>>& members,
    const std::unordered_map<std::string, std::shared_ptr<FuncType>>& methods)
    : members(members), methods(methods)
{}

std::unordered_map<std::string, std::shared_ptr<Type>> StructType::get_members() const {
    return members;
}

std::unordered_map<std::string, std::shared_ptr<FuncType>> StructType::get_methods() const {
    return methods;
}

bool StructType::equals(const std::shared_ptr<Type>& other) const {
    if (auto o = dynamic_cast<StructType*>(other.get())) {
        if (o->members.size() != members.size()) return false;
        for (auto& [k,v] : members) {
            auto it = o->members.find(k);
            if (it == o->members.end() || !v->equals(it->second)) return false;
        }
        return true;
    }
    return false;
}

void StructType::print() {
    std::cout << "StructType(members:{";
    for (auto& [n,t] : members) {
        std::cout << n << ":";
        t->print();
        std::cout << ", ";
    }
    std::cout << "}, methods:{";
    for (auto& [n,f] : methods) {
        std::cout << n << ":";
        f->print();
        std::cout << ", ";
    }
    std::cout << "})";
}

// ---------------------------
// PointerType
// ---------------------------

PointerType::PointerType(std::shared_ptr<Type> base)
    : base(std::move(base))
{}

std::shared_ptr<Type> PointerType::get_base() const {
    return base;
}

bool PointerType::equals(const std::shared_ptr<Type>& other) const {
    if (auto o = dynamic_cast<PointerType*>(other.get())) {
        return base->equals(o->base);
    }
    return false;
}

void PointerType::print() {
    std::cout << "PointerType(";
    if (base) base->print();
    else       std::cout << "nullptr";
    std::cout << ")";
}

// ---------------------------
// LValueType / RValueType
// ---------------------------

LValueType::LValueType(std::shared_ptr<Type> ref_to)
    : ref_to(std::move(ref_to))
{}

std::shared_ptr<Type> LValueType::get_referenced_type() const {
    return ref_to;
}

bool LValueType::equals(const std::shared_ptr<Type>& other) const {
    if (auto o = dynamic_cast<LValueType*>(other.get())) {
        return ref_to->equals(o->ref_to);
    }
    return false;
}

void LValueType::print() {
    std::cout << "LValue(";
    ref_to->print();
    std::cout << ")";
}

RValueType::RValueType(std::shared_ptr<Type> ref_to)
    : ref_to(std::move(ref_to))
{}

std::shared_ptr<Type> RValueType::get_referenced_type() const {
    return ref_to;
}

bool RValueType::equals(const std::shared_ptr<Type>& other) const {
    if (auto o = dynamic_cast<RValueType*>(other.get())) {
        return ref_to->equals(o->ref_to);
    }
    return false;
}

void RValueType::print() {
    std::cout << "RValue(";
    ref_to->print();
    std::cout << ")";
}

// ---------------------------
// ArrayType
// ---------------------------

ArrayType::ArrayType(std::shared_ptr<Type> base, expression size)
    : base(std::move(base)), size(size)
{}

std::shared_ptr<Type> ArrayType::get_base_type() const {
    return base;
}

expression ArrayType::get_size() const {
    return size;
}

bool ArrayType::equals(const std::shared_ptr<Type>& other) const {
    if (auto o = dynamic_cast<ArrayType*>(other.get())) {
        return base->equals(o->base);
    }
    return false;
}

void ArrayType::print() {
    std::cout << "Array(";
    base->print();
    std::cout << ")";
}

// ---------------------------
// ConstType
// ---------------------------

ConstType::ConstType(std::shared_ptr<Type> base)
    : base(std::move(base))
{}

std::shared_ptr<Type> ConstType::get_base() const {
    return base;
}

bool ConstType::equals(const std::shared_ptr<Type>& other) const {
    if (auto p = std::dynamic_pointer_cast<ConstType>(other)) {
        return base->equals(p->base);
    }
    return false;
}

void ConstType::print() {
    std::cout << "Const(";
    base->print();
    std::cout << ")";
}

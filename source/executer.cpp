
#include "executer.hpp"
#include <stdexcept>
#include <typeinfo>


struct BreakSignal : std::exception {};
struct ContinueSignal : std::exception {};
struct ReturnSignal : std::exception {
    std::any value;
    ReturnSignal(std::any v) : value(std::move(v)) {}
};

std::unordered_map<std::string, std::shared_ptr<Symbol>> Execute::default_types = {
    {"int",    std::make_shared<VarSymbol>(std::make_shared<IntegerType>(), std::any{})},
    {"float",  std::make_shared<VarSymbol>(std::make_shared<FloatType>(),   std::any{})},
    {"char",   std::make_shared<VarSymbol>(std::make_shared<CharType>(),    std::any{})},
    {"bool",   std::make_shared<VarSymbol>(std::make_shared<BoolType>(),    std::any{})},
    {"void",   std::make_shared<VarSymbol>(std::make_shared<VoidType>(),    std::any{})}
};

Execute::Execute() : symbolTable(std::make_shared<Scope>(nullptr)) { }


Execute::~Execute() { }


void Execute::execute(TranslationUnit& unit) {
    for (auto& node : unit.get_nodes()) {
        node->accept(*this);
    }

    std::shared_ptr<Symbol> mainBase;
    try {
        mainBase = symbolTable->match_global("main");
    } catch (...) {
        throw std::runtime_error("No 'main' function found");
    }
    auto mainSym = std::dynamic_pointer_cast<FuncSymbol>(mainBase);
    if (!mainSym) {
        throw std::runtime_error("'main' is not a function");
    }

    auto mainType = std::dynamic_pointer_cast<FuncType>(mainSym->type);
    if (!dynamic_cast<IntegerType*>(mainType->get_returnable_type().get())) {
        throw std::runtime_error("'main' must return int");
    }
    if (!mainType->get_args().empty()) {
        throw std::runtime_error("'main' should not take parameters");
    }

    int exitCode = 0;
    {
        auto savedScope = symbolTable;
        symbolTable = symbolTable->create_new_table(savedScope);

        try {
            mainSym->declaration->body->accept(*this);
        }
        catch (ReturnSignal& ret) {
            exitCode = std::any_cast<int>(ret.value);
        }

        symbolTable = savedScope;
    }


    (void)exitCode;
}


std::shared_ptr<Symbol> Execute::match_symbol(const std::string& token) {
    try {
        auto symbol = symbolTable->match_global(token);
        if (symbol) return symbol;
    } catch (...) {}
    auto it = default_types.find(token);
    if (it != default_types.end()) return it->second;
    throw std::runtime_error("Symbol or type '" + token + "' not found");
}

bool Execute::is_record_type(const std::shared_ptr<Type>& type) {
    return dynamic_cast<StructType*>(type.get()) != nullptr;
}

bool Execute::count_bool(std::any a, std::string& op, std::any b) {
    bool l = std::any_cast<bool>(a);
    bool r = std::any_cast<bool>(b);
    if (op == "&&") return l && r;
    if (op == "||") return l || r;
    return false;
}

std::shared_ptr<VarSymbol> Execute::binary_operation(
    std::shared_ptr<VarSymbol> lhsSym,
    std::string& op,
    std::shared_ptr<VarSymbol> rhsSym)
{
    auto lhsType = lhsSym->type;
    auto rhsType = rhsSym->type;
    auto lhsVal  = lhsSym->value;
    auto rhsVal  = rhsSym->value;


    if (op == "=") {
        if (auto arrElem = std::dynamic_pointer_cast<ArrayElementSymbol>(lhsSym)) {
            auto parent = arrElem->parentArray;
            int idx     = arrElem->index;
            auto &vec   = std::any_cast<std::vector<std::any>&>(parent->value);
            if (idx < 0 || idx >= static_cast<int>(vec.size()))
                throw std::runtime_error("binary_operation: array index out of range");
            vec[idx]       = rhsSym->value;
            arrElem->value = rhsSym->value;
            return arrElem;
        }
        lhsSym->value = rhsSym->value;
        return lhsSym;
    }

    // вспомогательный лямбда: является ли S указателем на VarSymbol или ArrayElementSymbol?
    auto isPointerToVar = [&](std::shared_ptr<VarSymbol> S)->bool {
        if (!S->type) return false;
        if (!dynamic_cast<PointerType*>(S->type.get())) return false;
        if (!S->value.has_value()) return false;
        const std::type_info &t = S->value.type();
        return (t == typeid(std::shared_ptr<VarSymbol>))
            || (t == typeid(std::shared_ptr<ArrayElementSymbol>));
    };

    // арифметика указателей: p + n или p - n
    if ((op == "+" || op == "-") && isPointerToVar(lhsSym)) {
        std::shared_ptr<VarSymbol> pointedVar;
        // достаём, куда указывает lhsSym
        if (lhsSym->value.type() == typeid(std::shared_ptr<VarSymbol>)) {
            pointedVar = std::any_cast<std::shared_ptr<VarSymbol>>(lhsSym->value);
        } else {
            auto arrPtr = std::any_cast<std::shared_ptr<ArrayElementSymbol>>(lhsSym->value);
            pointedVar = std::static_pointer_cast<VarSymbol>(arrPtr);
        }

        if (rhsVal.type() != typeid(int)) {
            throw std::runtime_error("pointer arithmetic: second operand must be int");
        }
        int offset = std::any_cast<int>(rhsVal);

        int baseIndex = 0;
        std::shared_ptr<VarSymbol> parentArraySym;
        if (auto arrElem = std::dynamic_pointer_cast<ArrayElementSymbol>(pointedVar)) {
            parentArraySym = arrElem->parentArray;
            baseIndex      = arrElem->index;
        } else {
            // указатель на "отдельную" переменную: допускаем только offset == 0
            if (offset != 0)
                throw std::runtime_error("pointer arithmetic goes out of bounds");
            parentArraySym = nullptr;
            baseIndex      = 0;
        }

        int newIndex = (op == "+") ? (baseIndex + offset) : (baseIndex - offset);

        if (parentArraySym) {
            auto &vec = std::any_cast<std::vector<std::any>&>(parentArraySym->value);
            if (newIndex < 0 || newIndex >= static_cast<int>(vec.size()))
                throw std::runtime_error("pointer arithmetic: out of array bounds");
            auto elemType   = std::static_pointer_cast<ArrayType>(parentArraySym->type)->get_base_type();
            auto newElem    = std::make_shared<ArrayElementSymbol>(elemType, vec[newIndex], parentArraySym, newIndex);
            auto newPtrType = std::make_shared<PointerType>(elemType);
            auto newPtrSym  = std::make_shared<VarSymbol>(newPtrType);
            newPtrSym->value = newElem;
            return newPtrSym;
        } else {
            // offset == 0 -> возвращаем тот же указатель
            return lhsSym;
        }
    }

    // --- 2) Вычитание указателей: p2 - p1 ---
    if (op == "-" && isPointerToVar(lhsSym) && isPointerToVar(rhsSym)) {
        std::shared_ptr<VarSymbol> leftPointed, rightPointed;
        if (lhsSym->value.type() == typeid(std::shared_ptr<VarSymbol>)) {
            leftPointed = std::any_cast<std::shared_ptr<VarSymbol>>(lhsSym->value);
        } else {
            auto arrPtrL = std::any_cast<std::shared_ptr<ArrayElementSymbol>>(lhsSym->value);
            leftPointed = std::static_pointer_cast<VarSymbol>(arrPtrL);
        }
        if (rhsSym->value.type() == typeid(std::shared_ptr<VarSymbol>)) {
            rightPointed = std::any_cast<std::shared_ptr<VarSymbol>>(rhsSym->value);
        } else {
            auto arrPtrR = std::any_cast<std::shared_ptr<ArrayElementSymbol>>(rhsSym->value);
            rightPointed = std::static_pointer_cast<VarSymbol>(arrPtrR);
        }

        if (auto arrElemL = std::dynamic_pointer_cast<ArrayElementSymbol>(leftPointed)) {
            if (auto arrElemR = std::dynamic_pointer_cast<ArrayElementSymbol>(rightPointed)) {
                if (arrElemL->parentArray == arrElemR->parentArray) {
                    int diff = arrElemL->index - arrElemR->index;
                    return std::make_shared<VarSymbol>(
                        std::make_shared<IntegerType>(), diff
                    );
                }
            }
        }
        throw std::runtime_error("pointer subtraction only valid for same array");
    }

    // cравнение указателей с nullptr или друг с другом
    if ((op == "==" || op == "!=") && (isPointerToVar(lhsSym) || isPointerToVar(rhsSym))) {
        // lhs == nullptr
        if (lhsVal.has_value() && lhsVal.type() == typeid(std::nullptr_t) && isPointerToVar(rhsSym)) {
            bool isNull = (std::any_cast<std::nullptr_t>(lhsVal) == nullptr);
            bool result = (op == "==") ? isNull : !isNull;
            return std::make_shared<VarSymbol>(std::make_shared<BoolType>(), result);
        }
        // rhs == nullptr
        if (rhsVal.has_value() && rhsVal.type() == typeid(std::nullptr_t) && isPointerToVar(lhsSym)) {
            bool isNull = (std::any_cast<std::nullptr_t>(rhsVal) == nullptr);
            bool result = (op == "==") ? isNull : !isNull;
            return std::make_shared<VarSymbol>(std::make_shared<BoolType>(), result);
        }
        // оба указателя
        if (isPointerToVar(lhsSym) && isPointerToVar(rhsSym)) {
            std::shared_ptr<VarSymbol> L, R;
            if (lhsVal.type() == typeid(std::shared_ptr<VarSymbol>)) {
                L = std::any_cast<std::shared_ptr<VarSymbol>>(lhsVal);
            } else {
                auto arrPtrL = std::any_cast<std::shared_ptr<ArrayElementSymbol>>(lhsVal);
                L = std::static_pointer_cast<VarSymbol>(arrPtrL);
            }
            if (rhsVal.type() == typeid(std::shared_ptr<VarSymbol>)) {
                R = std::any_cast<std::shared_ptr<VarSymbol>>(rhsVal);
            } else {
                auto arrPtrR = std::any_cast<std::shared_ptr<ArrayElementSymbol>>(rhsVal);
                R = std::static_pointer_cast<VarSymbol>(arrPtrR);
            }
            bool eq     = (L.get() == R.get());
            bool result = (op == "==") ? eq : !eq;
            return std::make_shared<VarSymbol>(std::make_shared<BoolType>(), result);
        }
        throw std::runtime_error("pointer comparison type mismatch");
    }

    //  композитные “+=, -=, *=, /=” и обычная арифметика для чисел
    if (op == "+=" || op == "-=" || op == "*=" || op == "/=") {
        std::any lhsV = lhsSym->value;
        std::any rhsV = rhsSym->value;
        auto toDouble = [&](const std::any& v)->double {
            if (!v.has_value())     return 0.0;
            if (v.type() == typeid(int))    return double(std::any_cast<int>(v));
            if (v.type() == typeid(bool))   return std::any_cast<bool>(v) ? 1.0 : 0.0;
            if (v.type() == typeid(char))   return double(std::any_cast<char>(v));
            if (v.type() == typeid(float))  return double(std::any_cast<float>(v));
            if (v.type() == typeid(double)) return         std::any_cast<double>(v);
            return 0.0;
        };
        auto toInt = [&](const std::any& v)->int {
            if (!v.has_value()) return 0;
            if (v.type() == typeid(int))    return         std::any_cast<int>(v);
            if (v.type() == typeid(bool))   return std::any_cast<bool>(v) ? 1 : 0;
            if (v.type() == typeid(char))   return int(std::any_cast<char>(v));
            if (v.type() == typeid(float))  return int(std::any_cast<float>(v));
            if (v.type() == typeid(double)) return int(std::any_cast<double>(v));
            return 0;
        };
        bool isFloatOp = (lhsV.type() == typeid(double) || rhsV.type() == typeid(double)
                       || lhsV.type() == typeid(float)  || rhsV.type() == typeid(float));
        std::any resultAny;
        std::shared_ptr<Type> resultType;
        if (op == "+=") {
            if (isFloatOp) {
                double l = toDouble(lhsV), r = toDouble(rhsV);
                resultAny = l + r;         resultType = std::make_shared<FloatType>();
            } else {
                int l = toInt(lhsV), r = toInt(rhsV);
                resultAny = l + r;         resultType = std::make_shared<IntegerType>();
            }
        }
        else if (op == "-=") {
            if (isFloatOp) {
                double l = toDouble(lhsV), r = toDouble(rhsV);
                resultAny = l - r;         resultType = std::make_shared<FloatType>();
            } else {
                int l = toInt(lhsV), r = toInt(rhsV);
                resultAny = l - r;         resultType = std::make_shared<IntegerType>();
            }
        }
        else if (op == "*=") {
            if (isFloatOp) {
                double l = toDouble(lhsV), r = toDouble(rhsV);
                resultAny = l * r;         resultType = std::make_shared<FloatType>();
            } else {
                int l = toInt(lhsV), r = toInt(rhsV);
                resultAny = l * r;         resultType = std::make_shared<IntegerType>();
            }
        }
        else { // "/="
            if (isFloatOp) {
                double l = toDouble(lhsV), r = toDouble(rhsV);
                if (r == 0.0) throw std::runtime_error("division by zero");
                resultAny = l / r;        resultType = std::make_shared<FloatType>();
            } else {
                int l = toInt(lhsV), r = toInt(rhsV);
                if (r == 0) throw std::runtime_error("division by zero");
                resultAny = l / r;        resultType = std::make_shared<IntegerType>();
            }
        }
        lhsSym->value = resultAny;
        lhsSym->type  = resultType;
        return lhsSym;
    }

    // обычная арифметика “+”, “-”, “*”, “/” для чисел
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        bool isFloatOp2 = (lhsVal.type() == typeid(double) || rhsVal.type() == typeid(double)
                        || lhsVal.type() == typeid(float)  || rhsVal.type() == typeid(float));
        std::any resultAny;
        std::shared_ptr<Type> resultType;
        auto toDouble = [&](const std::any& v)->double {
            if (!v.has_value())     return 0.0;
            if (v.type() == typeid(int))    return double(std::any_cast<int>(v));
            if (v.type() == typeid(bool))   return std::any_cast<bool>(v) ? 1.0 : 0.0;
            if (v.type() == typeid(char))   return double(std::any_cast<char>(v));
            if (v.type() == typeid(float))  return double(std::any_cast<float>(v));
            if (v.type() == typeid(double)) return         std::any_cast<double>(v);
            return 0.0;
        };
        auto toInt = [&](const std::any& v)->int {
            if (!v.has_value()) return 0;
            if (v.type() == typeid(int))    return         std::any_cast<int>(v);
            if (v.type() == typeid(bool))   return std::any_cast<bool>(v) ? 1 : 0;
            if (v.type() == typeid(char))   return int(std::any_cast<char>(v));
            if (v.type() == typeid(float))  return int(std::any_cast<float>(v));
            if (v.type() == typeid(double)) return int(std::any_cast<double>(v));
            return 0;
        };
        if (isFloatOp2) {
            double l = toDouble(lhsVal), r = toDouble(rhsVal);
            if (op == "+")       resultAny = l + r;
            else if (op == "-")  resultAny = l - r;
            else if (op == "*")  resultAny = l * r;
            else { // "/"
                if (r == 0.0) throw std::runtime_error("division by zero");
                resultAny = l / r;
            }
            resultType = std::make_shared<FloatType>();
        } else {
            int l = toInt(lhsVal), r = toInt(rhsVal);
            if (op == "+")      resultAny = l + r;
            else if (op == "-") resultAny = l - r;
            else if (op == "*") resultAny = l * r;
            else { // "/"
                if (r == 0) throw std::runtime_error("division by zero");
                resultAny = l / r;
            }
            resultType = std::make_shared<IntegerType>();
        }
        return std::make_shared<VarSymbol>(resultType, resultAny);
    }

    // ccравнения “<”, “>”, “<=”, “>=”, “==”, “!=” для чисел 
    if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        auto toDouble = [&](const std::any& v)->double {
            if (!v.has_value())     return 0.0;
            if (v.type() == typeid(int))    return double(std::any_cast<int>(v));
            if (v.type() == typeid(bool))   return std::any_cast<bool>(v) ? 1.0 : 0.0;
            if (v.type() == typeid(char))   return double(std::any_cast<char>(v));
            if (v.type() == typeid(float))  return double(std::any_cast<float>(v));
            if (v.type() == typeid(double)) return         std::any_cast<double>(v);
            return 0.0;
        };
        double l = toDouble(lhsVal), r = toDouble(rhsVal);
        bool cmp;
        if (op == "<")      cmp = (l < r);
        else if (op == ">") cmp = (l > r);
        else if (op == "<=") cmp = (l <= r);
        else if (op == ">=") cmp = (l >= r);
        else if (op == "==") cmp = (l == r);
        else /* "!=" */     cmp = (l != r);
        return std::make_shared<VarSymbol>(std::make_shared<BoolType>(), cmp);
    }

    throw std::runtime_error("unsupported binary operator: " + op);
}



std::shared_ptr<VarSymbol> Execute::unary_operation(std::shared_ptr<VarSymbol> baseSym,
                                                    std::string& op) {
    std::any v = baseSym->value;
    std::any resultAny;
    std::shared_ptr<Type> resultType = baseSym->type;

     if (op == "++") {
        if (v.type() == typeid(int)) {
            int x = std::any_cast<int>(v) + 1;
            baseSym->value = x;
            resultAny = x;
        }
        else if (v.type() == typeid(double)) {
            double x = std::any_cast<double>(v) + 1.0;
            baseSym->value = x;
            resultAny = x;
        }
        else {
            throw std::runtime_error("unsupported operand type for prefix ++");
        }
        // т ип остается тем же, что и у baseSym
        return std::make_shared<VarSymbol>(resultType, resultAny);
    }
    if (op == "--") {
        if (v.type() == typeid(int)) {
            int x = std::any_cast<int>(v) - 1;
            baseSym->value = x;
            resultAny = x;
        }
        else if (v.type() == typeid(double)) {
            double x = std::any_cast<double>(v) - 1.0;
            baseSym->value = x;
            resultAny = x;
        }
        else {
            throw std::runtime_error("unsupported operand type for prefix --");
        }
        return std::make_shared<VarSymbol>(resultType, resultAny);
    }

    if (op == "+") {
        if (v.type() == typeid(int))      resultAny = +std::any_cast<int>(v);
        else if (v.type() == typeid(double)) resultAny = +std::any_cast<double>(v);
        else throw std::runtime_error("unsupported operand for unary +");
    }
    else if (op == "-") {
        if (v.type() == typeid(int))      resultAny = -std::any_cast<int>(v);
        else if (v.type() == typeid(double)) resultAny = -std::any_cast<double>(v);
        else throw std::runtime_error("unsupported operand for unary -");
    }
    else if (op == "!") {
        bool bv;
        if (v.type() == typeid(bool))        bv = std::any_cast<bool>(v);
        else if (v.type() == typeid(int))    bv = (std::any_cast<int>(v) != 0);
        else if (v.type() == typeid(double)) bv = (std::any_cast<double>(v) != 0.0);
        else throw std::runtime_error("invalid operand for '!'");
        resultAny = !bv;
        resultType = std::make_shared<BoolType>();
    }
    else {
        throw std::runtime_error("unsupported unary operator: " + op);
    }

    return std::make_shared<VarSymbol>(resultType, resultAny);
}

std::shared_ptr<VarSymbol> Execute::postfix_operation(std::shared_ptr<VarSymbol> baseSym,
                                                      std::string& op) {
    // берём текущее значение из baseSym
    std::any oldVal = baseSym->value;
    std::any newVal;

    
    if (oldVal.type() == typeid(int)) {
        int x = std::any_cast<int>(oldVal);
        if (op == "++")      newVal = x + 1;
        else if (op == "--") newVal = x - 1;
        else throw std::runtime_error("unsupported postfix operator: " + op);
    }
    else if (oldVal.type() == typeid(double)) {
        double x = std::any_cast<double>(oldVal);
        if (op == "++")      newVal = x + 1.0;
        else if (op == "--") newVal = x - 1.0;
        else throw std::runtime_error("unsupported postfix operator: " + op);
    }
    else {
        throw std::runtime_error("unsupported type for postfix operator: " + op);
    }

    // обновляем значение в самой переменной
    baseSym->value = newVal;

    // возвращаем новый VarSymbol, содержащий прежнее значение (oldVal)
    return std::make_shared<VarSymbol>(baseSym->type, oldVal);
}


bool Execute::can_convert(const std::shared_ptr<Type>& from, const std::shared_ptr<Type>& to) {
    if (from->equals(to)) return true;
    if (dynamic_cast<Arithmetic*>(from.get()) && dynamic_cast<Arithmetic*>(to.get())) return true;
    return false;
}

void Execute::visit(ASTNode&) {
}

void Execute::visit(TranslationUnit& unit) {
    for (auto& node : unit.get_nodes()) {
        node->accept(*this);
    }
}


void Execute::visit(Declaration::SimpleDeclarator& node) {
}

void Execute::visit(Declaration::PtrDeclarator& node) {
}

void Execute::visit(Declaration::InitDeclarator& node) {
    
}
void Execute::visit(VarDeclaration& node) {
    for (auto& initDecl : node.declarator_list) {
        //vartype
        std::shared_ptr<Type> varType;
        std::any              initValue;

        if (node.type == "auto") {
          
            if (!initDecl->initializer) {
                throw std::runtime_error("auto‐declaration requires an initializer");
            }
            initDecl->initializer->accept(*this);
            auto rhsSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
            if (!rhsSym) {
                throw std::runtime_error("initializer for auto is not a VarSymbol");
            }
            varType   = rhsSym->type;
            initValue = rhsSym->value;
        }
        else {
            // явный тип: либо базовый, либо структурный
            std::shared_ptr<Symbol> typeSym;
            try {
                typeSym = match_symbol(node.type);
            } catch (...) {
                throw std::runtime_error("Symbol or type '" + node.type + "' not found");
            }
            varType = typeSym->type;

            // если есть инициализатор, вычисляем значение справа
            if (initDecl->initializer) {
                initDecl->initializer->accept(*this);
                auto rhsSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
                if (!rhsSym) {
                    throw std::runtime_error("initializer is not a VarSymbol");
                }
                initValue = rhsSym->value;
            }
            else {
                // default-инициализация 
                if (dynamic_cast<IntegerType*>(varType.get())) {
                    initValue = int(0);
                }
                else if (dynamic_cast<FloatType*>(varType.get())) {
                    initValue = double(0.0);
                }
                else if (dynamic_cast<BoolType*>(varType.get())) {
                    initValue = false;
                }
                else if (dynamic_cast<CharType*>(varType.get())) {
                    initValue = char(0);
                }
                else {
                    // Оставляем пустое std::any — если далее не придёт StructType
                    initValue = std::any{};
                }
            }
        }

        //ptrdeclarator -> pointertype
        if (dynamic_cast<Declaration::PtrDeclarator*>(initDecl->declarator.get())) {
            varType   = std::make_shared<PointerType>(varType);
        }


        // если varType — StructType, создаём новый экземпляр
        if (auto structT = std::dynamic_pointer_cast<StructType>(varType)) {
            // найдем "шаблонный" StructSymbol для node.type
            std::shared_ptr<Symbol> tmplSymAny;
            try {
                tmplSymAny = symbolTable->match_global(node.type);
            } catch (...) {
                throw std::runtime_error("Internal error: StructSymbol not found for '" + node.type + "'");
            }
            auto tmplStruct = std::dynamic_pointer_cast<StructSymbol>(tmplSymAny);
            if (!tmplStruct) {
                throw std::runtime_error("Internal error: symbol '" + node.type + "' is not a StructSymbol");
            }

            // скопировать все VarSymbol члены с дефолтными значениями
            std::unordered_map<std::string, std::shared_ptr<Symbol>> instance_members;
            for (auto& kv : tmplStruct->members) {
                if (auto fld = std::dynamic_pointer_cast<VarSymbol>(kv.second)) {
                    // новый VarSymbol того же типа, со значением по умолчанию
                    auto copyVar = std::make_shared<VarSymbol>(fld->type);
                    // если тип поля — int, float, bool или char, сразу задаем 0/0.0/false/'\0'
                    if (dynamic_cast<IntegerType*>(fld->type.get())) {
                        copyVar->value = int(0);
                    }
                    else if (dynamic_cast<FloatType*>(fld->type.get())) {
                        copyVar->value = double(0.0);
                    }
                    else if (dynamic_cast<BoolType*>(fld->type.get())) {
                        copyVar->value = false;
                    }
                    else if (dynamic_cast<CharType*>(fld->type.get())) {
                        copyVar->value = char(0);
                    }
                    // для любых других (например, вложенных структур) оставляем std::any{}
                    instance_members[kv.first] = copyVar;
                }
                else if (auto mtd = std::dynamic_pointer_cast<FuncSymbol>(kv.second)) {
                    // для методов копируем ссылку, их тела будут выполняться при вызове
                    instance_members[kv.first] = mtd;
                }
            }

            // создать сам StructSymbol-экземпляр
            auto instanceStruct = std::make_shared<StructSymbol>(
                std::static_pointer_cast<StructType>(varType),
                instance_members
            );
            initValue = instanceStruct;
        }

       // регистрируем глобальныую переменную 
        const auto& varName = initDecl->declarator->name;
        if (!symbolTable->contains_symbol(varName)) {
            auto newVar = std::make_shared<VarSymbol>(varType);
            symbolTable->push_symbol(varName, newVar);
        }


        // получаем уже существующий VarSymbol и кладём initValue
   
        auto baseSym = symbolTable->match_global(varName);
        auto existingVarSym = std::dynamic_pointer_cast<VarSymbol>(baseSym);
        if (!existingVarSym) {
            throw std::runtime_error(
                "Internal error: variable '" + varName +
                "' was expected to be registered but wasn't found"
            );
        }
        existingVarSym->value = initValue;
        current_value = existingVarSym;
    }
}


void Execute::visit(ParameterDeclaration& node) {
    // определяем тип и значение так же, как раньше:
    auto typeName = node.type;
    std::shared_ptr<Type> pType = match_symbol(typeName)->type;
    auto name = node.init_declarator->declarator->name;

    std::any value{};
    if (node.init_declarator->initializer) {
        node.init_declarator->initializer->accept(*this);
        value = std::dynamic_pointer_cast<VarSymbol>(current_value)->value;
    }

    //  не создаём новый VarSymbol, а берём тот, что уже создал Analyzer:
    auto baseSym = symbolTable->match_global(name);
    auto existingParam = std::dynamic_pointer_cast<VarSymbol>(baseSym);
    if (!existingParam) {
        throw std::runtime_error(
            "Internal error: параметр '" + name +
            "' не найден в таблице, хотя Analyzer должен был его зарегистрировать"
        );
    }
    //  обновляем его значение
    existingParam->value = value;
    current_value = existingParam;
}


void Execute::visit(FuncDeclaration& node) {
    // попытка найти FuncSymbol в текущем скоупе
    std::shared_ptr<Symbol> baseSym;
    bool exists = true;
    try {
        baseSym = symbolTable->match_global(node.declarator->name);
    } catch (...) {
        exists = false;
    }

    // если символ уже есть (например, метод struct или ранее зарегистрированная функция) - просто выходим
    if (exists) {
        current_value = nullptr;
        return;
    }

    // иначе — это топ-левел (глобальная) функция. Создаём FuncType и FuncSymbol и пушим его.
    // определяем возвращаемый тип
    std::shared_ptr<Type> retType;
    if (node.type == "auto") {
        retType = std::make_shared<VoidType>();
    } else {
        auto retSym = match_symbol(node.type);
        retType = retSym->type;
        if (node.is_const) {
            retType = std::make_shared<ConstType>(retType);
        }
    }

    //собираем типы параметров
    std::vector<std::shared_ptr<Type>> argTypes;
    for (auto& p : node.args) {
        auto pSym = match_symbol(p->type);
        auto pType = pSym->type;
        argTypes.push_back(pType);
    }

    // создаём FuncType и FuncSymbol
    auto fType = std::make_shared<FuncType>(retType, argTypes, node.is_readonly);
    auto fSym = std::make_shared<FuncSymbol>(fType, argTypes, node.is_readonly);
    fSym->declaration = &node;

    
    symbolTable->push_symbol(node.declarator->name, fSym);
    current_value = nullptr;
}


void Execute::visit(StructDeclaration& node) {
    // находим уже существующий StructSymbol который создал Analyzer
    std::shared_ptr<Symbol> baseSym;
    try {
        baseSym = symbolTable->match_global(node.name);
    }
    catch (...) {
        throw std::runtime_error("Internal error: StructSymbol не найден для структуры " + node.name);
    }
    auto structSym = std::dynamic_pointer_cast<StructSymbol>(baseSym);
    if (!structSym) {
        throw std::runtime_error("Internal error: символ " + node.name + " не является StructSymbol");
    }

    auto member_symbols = structSym->members;

    // открываем вложенный скоп и заселяем в него
    auto savedScope = symbolTable;
    symbolTable = symbolTable->create_new_table(savedScope);
    for (auto& kv : member_symbols) {
        symbolTable->push_symbol(kv.first, kv.second);
    }

  
    for (auto& m : node.members) {
        if (auto fldDecl = dynamic_cast<VarDeclaration*>(m.get())) {
            fldDecl->accept(*this);
        }
        else if (auto mtdDecl = dynamic_cast<FuncDeclaration*>(m.get())) {

            continue;
        }
        else {
            m->accept(*this);
        }
    }

   
    symbolTable = savedScope;
    current_value = structSym;
}

void Execute::visit(ArrayDeclaration& node) {
    
    node.size->accept(*this);
    int sz = std::any_cast<int>(
        std::dynamic_pointer_cast<VarSymbol>(current_value)->value
    );

    auto elemType = match_symbol(node.type)->type;

    std::vector<std::any> data(sz);
    for (int i = 0; i < sz; ++i) {
        if      (dynamic_cast<IntegerType*>(elemType.get())) data[i] = int(0);
        else if (dynamic_cast<FloatType*>(elemType.get()))   data[i] = double(0.0);
        else if (dynamic_cast<BoolType*>(elemType.get()))    data[i] = false;
        else if (dynamic_cast<CharType*>(elemType.get()))    data[i] = char16_t(0);
        else                                                  data[i] = std::any{};
    }

    if (!node.initializer_list.empty()) {
        int initCount = static_cast<int>(node.initializer_list.size());
        int limit = std::min(sz, initCount);
        for (int i = 0; i < limit; ++i) {
            node.initializer_list[i]->accept(*this);
            auto valSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
            if (!valSym) {
                throw std::runtime_error(
                    "array-init: initializer is not a VarSymbol"
                );
            }
            data[i] = valSym->value;
        }
    }

    // регистрируем или находим VarSymbol для именованного массива:
    std::shared_ptr<VarSymbol> arraySym;
    bool alreadyExists = true;
    try {
        auto sym = symbolTable->match_global(node.name);
        arraySym = std::dynamic_pointer_cast<VarSymbol>(sym);
        if (!arraySym) {
            throw std::runtime_error(
                "Symbol '" + node.name + "' is not a variable"
            );
        }
    } catch (...) {
        alreadyExists = false;
    }

    if (!alreadyExists) {
        auto arrayType = std::make_shared<ArrayType>(elemType, node.size);
        auto newArrSym = std::make_shared<VarSymbol>(arrayType);
        symbolTable->push_symbol(node.name, newArrSym);
        arraySym = newArrSym;
    }

    arraySym->value = std::move(data);
    current_value = arraySym;
}

void Execute::visit(NameSpaceDeclaration& node) {
    auto savedScope = symbolTable;
    symbolTable = symbolTable->create_new_table(savedScope);
    for (auto& decl : node.declarations) {
        decl->accept(*this);
    }
    auto nsSym = std::make_shared<NamespaceSymbol>(symbolTable);
    symbolTable = savedScope;
    symbolTable->push_symbol(node.name, nsSym);
    current_value = nsSym;
}



void Execute::visit(CompoundStatement& node) {
    auto savedScope = symbolTable;
    symbolTable = symbolTable->create_new_table(savedScope);
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
    symbolTable = savedScope;
}

void Execute::visit(DeclarationStatement& node) {
    node.declaration->accept(*this);
}

void Execute::visit(ExpressionStatement& node) {
    node.expression->accept(*this);
}

void Execute::visit(ConditionalStatement& node) {
    node.if_branch.first->accept(*this);
    auto cv = std::dynamic_pointer_cast<VarSymbol>(current_value)->value;
    bool cond = (cv.type() == typeid(bool) ? std::any_cast<bool>(cv) : std::any_cast<int>(cv) != 0);
    if (cond) {
        node.if_branch.second->accept(*this);
    } else if (node.else_branch) {
        node.else_branch->accept(*this);
    }
}

void Execute::visit(WhileStatement& node) {
    while (true) {
        node.condition->accept(*this);
        auto cv = std::dynamic_pointer_cast<VarSymbol>(current_value)->value;
        bool cond = (cv.type() == typeid(bool) ? std::any_cast<bool>(cv) : std::any_cast<int>(cv) != 0);
        if (!cond) break;
        try {
            node.statement->accept(*this);
        }
        catch (ContinueSignal&) {
            continue;
        }
        catch (BreakSignal&) {
            break;
        }
    }
}

void Execute::visit(ForStatement& node) {
    if (node.initialization) {
        node.initialization->accept(*this);
    }
    while (true) {
        if (node.condition) {
            node.condition->accept(*this);
            auto cv = std::dynamic_pointer_cast<VarSymbol>(current_value)->value;
            bool cond = (cv.type() == typeid(bool) ? std::any_cast<bool>(cv) : std::any_cast<int>(cv) != 0);
            if (!cond) break;
        }
        try {
            node.body->accept(*this);
        }
        catch (ContinueSignal&) {
            // fallthrough to increment
        }
        catch (BreakSignal&) {
            break;
        }
        if (node.increment) {
            node.increment->accept(*this);
        }
    }
}

void Execute::visit(ReturnStatement& node) {
    if (node.expression) {
        node.expression->accept(*this);
        auto rv = std::dynamic_pointer_cast<VarSymbol>(current_value)->value;
        throw ReturnSignal{rv};
    } else {
        throw ReturnSignal{std::any{}};
    }
}

void Execute::visit(BreakStatement&) {
    throw BreakSignal{};
}

void Execute::visit(ContinueStatement&) {
    throw ContinueSignal{};
}

void Execute::visit(StructMemberAccessExpression& node) {
    node.base->accept(*this);
    auto varSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
    auto structSym = std::any_cast<std::shared_ptr<StructSymbol>>(varSym->value);
    auto memberName = node.member;
    if (auto fld = std::dynamic_pointer_cast<VarSymbol>(structSym->members.at(memberName))) {
        current_value = fld;
    } else if (auto mth = std::dynamic_pointer_cast<FuncSymbol>(structSym->members.at(memberName))) {
        current_value = mth;
    } else {
        throw std::runtime_error("no such member: " + memberName);
    }
}

void Execute::visit(DoWhileStatement& node) {
    do {
        node.statement->accept(*this);
        node.condition->accept(*this);
        auto cv = std::dynamic_pointer_cast<VarSymbol>(current_value)->value;
        bool cond = (cv.type() == typeid(bool) ? std::any_cast<bool>(cv) : std::any_cast<int>(cv) != 0);
        if (!cond) break;
    } while (true);
}

void Execute::visit(BinaryOperation& node) {
    node.lhs->accept(*this);
    auto lhsSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
    node.rhs->accept(*this);
    auto rhsSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
    current_value = binary_operation(lhsSym, node.op, rhsSym);
}


void Execute::visit(PrefixExpression& node) {
    // сначала вычисляем "внутреннее" выражение и получаем VarSymbol или ArrayElementSymbol
    node.base->accept(*this);
    auto baseSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
    if (!baseSym) {
        throw std::runtime_error("prefix: base is not a variable");
    }

    if (node.op == "&") {
        auto innerType = baseSym->type;
        auto ptrType   = std::make_shared<PointerType>(innerType);
        std::any ptrValue = baseSym; // «указатель» хранит shared_ptr<VarSymbol> или ArrayElementSymbol
        current_value = std::make_shared<VarSymbol>(ptrType, ptrValue);
        return;
    }

    if (node.op == "*") {
        auto pType = std::dynamic_pointer_cast<PointerType>(baseSym->type);
        if (!pType) throw std::runtime_error("cannot dereference non-pointer type");
        if (!baseSym->value.has_value()) throw std::runtime_error("invalid pointer value");

        std::shared_ptr<VarSymbol> pointedVar;
        if (baseSym->value.type() == typeid(std::shared_ptr<VarSymbol>)) {
            pointedVar = std::any_cast<std::shared_ptr<VarSymbol>>(baseSym->value);
        }
        else {
            auto arrPtr = std::any_cast<std::shared_ptr<ArrayElementSymbol>>(baseSym->value);
            pointedVar = std::static_pointer_cast<VarSymbol>(arrPtr);
        }

        if (!pointedVar) throw std::runtime_error("invalid pointer value");
        current_value = pointedVar;
        return;
    }

    if (node.op == "++" || node.op == "--") {
        if (baseSym->value.type() == typeid(int)) {
            int x = std::any_cast<int>(baseSym->value);
            if (node.op == "++")      x += 1;
            else                      x -= 1;
            baseSym->value = x;
            current_value = std::make_shared<VarSymbol>(baseSym->type, x);
            return;
        }
        if (baseSym->value.type() == typeid(double)) {
            double x = std::any_cast<double>(baseSym->value);
            if (node.op == "++")      x += 1.0;
            else                      x -= 1.0;
            baseSym->value = x;
            current_value = std::make_shared<VarSymbol>(baseSym->type, x);
            return;
        }
        throw std::runtime_error("unsupported operand for prefix " + node.op);
    }

    if (node.op == "+" || node.op == "-" || node.op == "!") {
        current_value = unary_operation(baseSym, node.op);
        return;
    }

    throw std::runtime_error("unsupported prefix operator: " + node.op);
}



void Execute::visit(PostfixIncrementExpression& node) {
    node.base->accept(*this);
    auto baseSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
    std::string op = "++";

    auto resultSym = postfix_operation(baseSym, op);

    current_value = resultSym;
}

void Execute::visit(PostfixDecrementExpression& node) {
    node.base->accept(*this);
    auto baseSym = std::dynamic_pointer_cast<VarSymbol>(current_value);

    std::string op = "--";

    auto resultSym = postfix_operation(baseSym, op);

    current_value = resultSym;
}



void Execute::visit(FunctionCallExpression& node) {
      if (auto ident = dynamic_cast<IdentifierExpression*>(node.base.get())) {
        if (ident->name == "print") {
            for (size_t i = 0; i < node.args.size(); ++i) {
                node.args[i]->accept(*this);
                auto vsym = std::dynamic_pointer_cast<VarSymbol>(current_value);
                if (!vsym) {
                    std::cout << "<<?>"; 
                }
                else {
                    // если указатель вывводим адрес
                    if (dynamic_cast<PointerType*>(vsym->type.get())) {
                        if (vsym->value.has_value() &&
                            vsym->value.type() == typeid(std::shared_ptr<VarSymbol>))
                        {
                            auto pointed = std::any_cast<std::shared_ptr<VarSymbol>>(vsym->value);
                            std::cout << pointed.get();
                        } else {
                            std::cout << "<ptr>";
                        }
                    }
                    else if (vsym->value.type() == typeid(std::string)) {
                        std::string s = std::any_cast<std::string>(vsym->value);
                        if (s.size() >= 2 && s.front() == '\"' && s.back() == '\"') {
                            s = s.substr(1, s.size() - 2);
                        }
                        std::cout << s;
                    }
                    else {
                        const auto& ti = vsym->value.type();
                        if (ti == typeid(int)) {
                            std::cout << std::any_cast<int>(vsym->value);
                        }
                        else if (ti == typeid(bool)) {
                            std::cout << (std::any_cast<bool>(vsym->value) ? "true" : "false");
                        }
                        else if (ti == typeid(double)) {
                            std::cout << std::any_cast<double>(vsym->value);
                        }
                        else if (ti == typeid(float)) {
                            std::cout << std::any_cast<float>(vsym->value);
                        }
                        else if (ti == typeid(char)) {
                            std::cout << std::any_cast<char>(vsym->value);
                        }
                        else {
                            std::cout << "<<?>";
                        }
                    }
                }

                if (i + 1 < node.args.size()) {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;

            current_value = std::make_shared<VarSymbol>(std::make_shared<IntegerType>(), 0);
            return;
        }
    }


    if (auto ident = dynamic_cast<IdentifierExpression*>(node.base.get())) {
        if (ident->name == "read") {
            if (node.args.size() != 1) {
                throw std::runtime_error("read() requires exactly one argument");
            }

            node.args[0]->accept(*this);
            auto targetSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
            auto arrElemSym = std::dynamic_pointer_cast<ArrayElementSymbol>(current_value);

            if (arrElemSym) {
               
                auto parentArr = arrElemSym->parentArray;  
                int idx       = arrElemSym->index;         
                auto &vec = std::any_cast<std::vector<std::any>&>(parentArr->value);

                auto elemType = std::dynamic_pointer_cast<ArrayType>(parentArr->type)->get_base_type();


                if (dynamic_cast<IntegerType*>(elemType.get())) {
                    int v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read an integer from stdin");
                    }
                    vec[idx] = v;
                    arrElemSym->value = v; 
                }
                else if (dynamic_cast<FloatType*>(elemType.get())) {
                    double v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read a float from stdin");
                    }
                    vec[idx] = v;
                    arrElemSym->value = v;
                }
                else if (dynamic_cast<CharType*>(elemType.get())) {
                    char v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read a char from stdin");
                    }
                    vec[idx] = v;
                    arrElemSym->value = v;
                }
                else if (dynamic_cast<BoolType*>(elemType.get())) {
                    bool v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read a bool from stdin");
                    }
                    vec[idx] = v;
                    arrElemSym->value = v;
                }
                else if (dynamic_cast<StringType*>(elemType.get())) {
                    std::string v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read a string from stdin");
                    }
                    vec[idx] = v;
                    arrElemSym->value = v;
                }
                else {
                    throw std::runtime_error("read(): unsupported array-element type");
                }

             
                current_value = arrElemSym;
                return;
            }

            // если это просто переменная, то читаем в неё значение
            if (targetSym) {
                auto varType = targetSym->type;

                if (dynamic_cast<IntegerType*>(varType.get())) {
                    int v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read an integer from stdin");
                    }
                    targetSym->value = v;
                }
                else if (dynamic_cast<FloatType*>(varType.get())) {
                    double v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read a float from stdin");
                    }
                    targetSym->value = v;
                }
                else if (dynamic_cast<CharType*>(varType.get())) {
                    char v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read a char from stdin");
                    }
                    targetSym->value = v;
                }
                else if (dynamic_cast<BoolType*>(varType.get())) {
                    bool v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read a bool from stdin");
                    }
                    targetSym->value = v;
                }
                else if (dynamic_cast<StringType*>(varType.get())) {
                    std::string v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read a string from stdin");
                    }
                    targetSym->value = v;
                }
                else {
                    throw std::runtime_error("read(): unsupported variable type");
                }
                current_value = targetSym;
                return;
            }

            throw std::runtime_error("read(): argument must be a variable or array element");
        }
    }


    

    std::vector<std::any> argVals;
    for (auto& argExpr : node.args) {
        argExpr->accept(*this);
        auto vsym = std::dynamic_pointer_cast<VarSymbol>(current_value);
        argVals.push_back(vsym->value);
    }

    // вызов метода структуры
    if (auto mexpr = dynamic_cast<StructMemberAccessExpression*>(node.base.get())) {
        mexpr->accept(*this);
        auto funcSym = std::dynamic_pointer_cast<FuncSymbol>(current_value);
        if (!funcSym) {
            throw std::runtime_error("expression is not a method");
        }
        auto funcType = std::dynamic_pointer_cast<FuncType>(funcSym->type);
        const auto& paramTypes = funcType->get_args();
        const auto& paramDecls = funcSym->declaration->args;
        if (paramTypes.size() != argVals.size()) {
            throw std::runtime_error("argument count mismatch in method call");
        }

        auto savedScope = symbolTable;
        symbolTable = symbolTable->create_new_table(savedScope);

        for (size_t i = 0; i < paramTypes.size(); ++i) {
            const auto& pname = paramDecls[i]->init_declarator->declarator->name;
            auto pType = paramTypes[i];
            auto pValue = argVals[i];
            auto vsym = std::make_shared<VarSymbol>(pType, pValue);
            symbolTable->push_symbol(pname, vsym);
        }

        try {
            funcSym->declaration->body->accept(*this);
            auto retType = funcType->get_returnable_type();
            current_value = std::make_shared<VarSymbol>(retType, std::any{});
        }
        catch (ReturnSignal& ret) {
            current_value = std::make_shared<VarSymbol>(
                funcType->get_returnable_type(),
                std::move(ret.value)
            );
        }
        symbolTable = savedScope;
        return;
    }

    // свободная функция
    if (auto ident = dynamic_cast<IdentifierExpression*>(node.base.get())) {
        auto baseSym = symbolTable->match_global(ident->name);
        auto funcSym = std::dynamic_pointer_cast<FuncSymbol>(baseSym);
        if (!funcSym) {
            throw std::runtime_error("Undefined function: " + ident->name);
        }

        auto funcType = std::dynamic_pointer_cast<FuncType>(funcSym->type);
        const auto& paramTypes = funcType->get_args();
        const auto& paramDecls = funcSym->declaration->args;
        if (paramTypes.size() != argVals.size()) {
            throw std::runtime_error("argument count mismatch");
        }

        auto savedScope = symbolTable;
        symbolTable = symbolTable->create_new_table(savedScope);

        for (size_t i = 0; i < paramTypes.size(); ++i) {
            const auto& pname = paramDecls[i]->init_declarator->declarator->name;
            auto pType = paramTypes[i];
            auto pValue = argVals[i];
            auto vsym = std::make_shared<VarSymbol>(pType, pValue);
            symbolTable->push_symbol(pname, vsym);
        }

        try {
            funcSym->declaration->body->accept(*this);
            auto retType = funcType->get_returnable_type();
            current_value = std::make_shared<VarSymbol>(retType, std::any{});
        }
        catch (ReturnSignal& ret) {
            current_value = std::make_shared<VarSymbol>(
                funcType->get_returnable_type(),
                std::move(ret.value)
            );
        }

        symbolTable = savedScope;
        return;
    }

    throw std::runtime_error("FunctionCallExpression: base is not an identifier or method");
}

void Execute::visit(SubscriptExpression& node) {
    node.base->accept(*this);
    auto arrSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
    if (!arrSym) {
        throw std::runtime_error("subscript: base is not a variable");
    }
    auto &vec = std::any_cast<std::vector<std::any>&>(arrSym->value);

    node.index->accept(*this);
    auto idxSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
    if (!idxSym) {
        throw std::runtime_error("subscript: index is not a variable");
    }
    int idx = std::any_cast<int>(idxSym->value);

    if (idx < 0 || idx >= static_cast<int>(vec.size())) {
        throw std::runtime_error("subscript: array index out of range");
    }

    std::any elemVal = vec[idx];
    auto arrType = std::dynamic_pointer_cast<ArrayType>(arrSym->type);
    if (!arrType) {
        throw std::runtime_error("subscript: variable is not an array");
    }
    auto elemType = arrType->get_base_type();

    auto elemSym = std::make_shared<ArrayElementSymbol>(elemType, elemVal, arrSym, idx);
    current_value = elemSym;
}


void Execute::visit(IntLiteral& node) {
    current_value = std::make_shared<VarSymbol>(std::make_shared<IntegerType>(), node.value);
}

void Execute::visit(FloatLiteral& node) {
    current_value = std::make_shared<VarSymbol>(std::make_shared<FloatType>(), node.value);
}

void Execute::visit(CharLiteral& node) {
    current_value = std::make_shared<VarSymbol>(std::make_shared<CharType>(), node.value);
}

void Execute::visit(StringLiteral& node) {
    current_value = std::make_shared<VarSymbol>(std::make_shared<StringType>(), node.value);
}

void Execute::visit(BoolLiteral& node) {
    current_value = std::make_shared<VarSymbol>(std::make_shared<BoolType>(), node.value);
}

void Execute::visit(NullPtrLiteral& node) {
    current_value = std::make_shared<VarSymbol>(std::make_shared<NullPtrType>());
}

void Execute::visit(IdentifierExpression& node) {
    // cначала находим символ в таблице:
    auto sym = symbolTable->match_global(node.name);
    auto varSym = std::dynamic_pointer_cast<VarSymbol>(sym);
    if (!varSym) {
        // если это не VarSymbol просто вернём его "как есть"
        current_value = sym;
        return;
    }

    // если тип VarSymbol - ArrayType, то при обращении к имени массива
    // мы делаем «decay» в указатель на первый элемент:
    if (auto arrType = std::dynamic_pointer_cast<ArrayType>(varSym->type)) {
        // получаем ссылку на вектор-данных:
        auto& vec = std::any_cast<std::vector<std::any>&>(varSym->value);
        // если массив пустой (теоретически), invalid pointer:
        if (vec.empty()) {
              std::cout << "errror 3";
            throw std::runtime_error("invalid pointer value");
        }
        auto elemType = arrType->get_base_type();
        std::any firstVal = vec[0];
        auto elemSymbol0 = std::make_shared<ArrayElementSymbol>(
            elemType,
            firstVal,
            varSym,
            /*index=*/0
        );
        // создаём новый VarSymbol типа «pointer to elemType» и кладём в него указатель на elemSymbol0:
        auto ptrType = std::make_shared<PointerType>(elemType);
        auto ptrSym  = std::make_shared<VarSymbol>(ptrType);
        ptrSym->value = elemSymbol0;
        current_value = ptrSym;
        return;
    }

    // если тип VarSymbol - не массив, просто возвращаем его
    current_value = varSym;
}


void Execute::visit(ParenthesizedExpression& node) {
    node.expression->accept(*this);
}

void Execute::visit(TernaryExpression& node) {
    node.condition->accept(*this);
    auto cv = std::dynamic_pointer_cast<VarSymbol>(current_value)->value;
    bool cond = (cv.type() == typeid(bool) ? std::any_cast<bool>(cv) : std::any_cast<int>(cv) != 0);
    if (cond) {
        node.true_expr->accept(*this);
    } else {
        node.false_expr->accept(*this);
    }
}

void Execute::visit(SizeOfExpression& node) {
    if (node.is_type) {
        int sz = sizeof(void*);
        current_value = std::make_shared<VarSymbol>(std::make_shared<IntegerType>(), sz);
    } else {
        node.expression->accept(*this);
        int sz = sizeof(void*);
        current_value = std::make_shared<VarSymbol>(std::make_shared<IntegerType>(), sz);
    }
}

void Execute::visit(NameSpaceAcceptExpression& node) {
    if (auto baseId = dynamic_cast<IdentifierExpression*>(node.base.get())) {
        auto nsSym = std::dynamic_pointer_cast<NamespaceSymbol>(
            symbolTable->match_global(baseId->name)
        );
        auto saved = symbolTable;
        symbolTable = nsSym->scope;
        auto member = symbolTable->match_global(node.name);
        current_value = member;
        symbolTable = saved;
    }
}

void Execute::visit(StaticAssertStatement& node) {

}


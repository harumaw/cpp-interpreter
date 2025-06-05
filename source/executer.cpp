
#include "executer.hpp"
#include <stdexcept>
#include <typeinfo>

// Собственные «сигнальные» исключения для управления потоком
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


// === 2) Метод execute(...) ===
void Execute::execute(TranslationUnit& unit) {
    // Регистрация объявлений (функции, переменные), но без выполнения тел функций
    for (auto& node : unit.get_nodes()) {
        node->accept(*this);
    }

    // Ищем функцию main
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

    // Проверяем сигнатуру main(): int main()
    auto mainType = std::dynamic_pointer_cast<FuncType>(mainSym->type);
    if (!dynamic_cast<IntegerType*>(mainType->get_returnable_type().get())) {
        throw std::runtime_error("'main' must return int");
    }
    if (!mainType->get_args().empty()) {
        throw std::runtime_error("'main' should not take parameters");
    }

    // Выполняем тело main в новом scope, ловя ReturnSignal
    int exitCode = 0;
    {
        auto savedScope = symbolTable;
        symbolTable = symbolTable->create_new_table(savedScope);

        try {
            // собственно «выполнение» тела main()
            mainSym->declaration->body->accept(*this);
        }
        catch (ReturnSignal& ret) {
            exitCode = std::any_cast<int>(ret.value);
        }

        symbolTable = savedScope;
    }

    // exitCode можно использовать дальше по желанию
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

std::shared_ptr<VarSymbol> Execute::binary_operation(std::shared_ptr<VarSymbol> lhsSym,
                                                     std::string& op,
                                                     std::shared_ptr<VarSymbol> rhsSym) {
    if (op == "=") {
        // 1.1) Проверим, не ArrayElementSymbol ли lhsSym
        if (auto arrElem = std::dynamic_pointer_cast<ArrayElementSymbol>(lhsSym)) {
            auto parent = arrElem->parentArray;    // VarSymbol массива
            int idx    = arrElem->index;           // индекс внутри массива

            // Получаем ссылку на in-place вектор из parent->value
            auto &vec = std::any_cast<std::vector<std::any>&>(parent->value);

            // Проверка границ (на всякий случай)
            if (idx < 0 || idx >= static_cast<int>(vec.size())) {
                throw std::runtime_error("binary_operation: array index out of range");
            }

            // Кладём внутрь vec новое значение
            vec[idx] = rhsSym->value;

            // Обновим сам lhsSym->value (чтобы его потом можно было прочитать, если нужно)
            arrElem->value = rhsSym->value;

            // Возвращаем как результат присваивания сам объект arrElem
            return arrElem;
        }

        // 1.2) Если же это обычное присваивание переменной x = y
        lhsSym->value = rhsSym->value;
        return lhsSym;
    }

    // --------------------------------
    // 2) Композитные "+=", "-=", "*=", "/="
    // --------------------------------
    if (op == "+=" || op == "-=" || op == "*=" || op == "/=") {
        std::any lhsVal = lhsSym->value;
        std::any rhsVal = rhsSym->value;

        auto toDouble = [&](const std::any& v) -> double {
            if (!v.has_value()) return 0.0;
            if (v.type() == typeid(int))    return double(std::any_cast<int>(v));
            if (v.type() == typeid(bool))   return std::any_cast<bool>(v) ? 1.0 : 0.0;
            if (v.type() == typeid(char))   return double(std::any_cast<char>(v));
            if (v.type() == typeid(float))  return double(std::any_cast<float>(v));
            if (v.type() == typeid(double)) return         std::any_cast<double>(v);
            return 0.0;
        };

        auto toInt = [&](const std::any& v) -> int {
            if (!v.has_value()) return 0;
            if (v.type() == typeid(int))    return         std::any_cast<int>(v);
            if (v.type() == typeid(bool))   return std::any_cast<bool>(v) ? 1 : 0;
            if (v.type() == typeid(char))   return int(std::any_cast<char>(v));
            if (v.type() == typeid(float))  return int(std::any_cast<float>(v));
            if (v.type() == typeid(double)) return int(std::any_cast<double>(v));
            return 0;
        };

        bool isFloatOp = (lhsVal.type() == typeid(double) || rhsVal.type() == typeid(double)
                       || lhsVal.type() == typeid(float)  || rhsVal.type() == typeid(float));

        std::any resultAny;
        std::shared_ptr<Type> resultType;

        if (op == "+=") {
            if (isFloatOp) {
                double l = toDouble(lhsVal), r = toDouble(rhsVal);
                resultAny = l + r;         resultType = std::make_shared<FloatType>();
            } else {
                int l = toInt(lhsVal), r = toInt(rhsVal);
                resultAny = l + r;         resultType = std::make_shared<IntegerType>();
            }
        }
        else if (op == "-=") {
            if (isFloatOp) {
                double l = toDouble(lhsVal), r = toDouble(rhsVal);
                resultAny = l - r;         resultType = std::make_shared<FloatType>();
            } else {
                int l = toInt(lhsVal), r = toInt(rhsVal);
                resultAny = l - r;         resultType = std::make_shared<IntegerType>();
            }
        }
        else if (op == "*=") {
            if (isFloatOp) {
                double l = toDouble(lhsVal), r = toDouble(rhsVal);
                resultAny = l * r;         resultType = std::make_shared<FloatType>();
            } else {
                int l = toInt(lhsVal), r = toInt(rhsVal);
                resultAny = l * r;         resultType = std::make_shared<IntegerType>();
            }
        }
        else { // "/="
            if (isFloatOp) {
                double l = toDouble(lhsVal), r = toDouble(rhsVal);
                if (r == 0.0) throw std::runtime_error("division by zero");
                resultAny = l / r;        resultType = std::make_shared<FloatType>();
            } else {
                int l = toInt(lhsVal), r = toInt(rhsVal);
                if (r == 0) throw std::runtime_error("division by zero");
                resultAny = l / r;        resultType = std::make_shared<IntegerType>();
            }
        }

        // Сохраняем новое значение в lhsSym
        lhsSym->value = resultAny;
        lhsSym->type  = resultType;
        return lhsSym;
    }

    // --------------------------------
    // 3) Прочие бинарные операции (+, -, *, /, <, >, ==, !=, &&, || и т.п.)
    // --------------------------------
    std::any lhsVal  = lhsSym->value;
    std::any rhsVal  = rhsSym->value;

    auto toDouble = [&](const std::any& v) -> double {
        if (!v.has_value()) return 0.0;
        if (v.type() == typeid(int))    return double(std::any_cast<int>(v));
        if (v.type() == typeid(bool))   return std::any_cast<bool>(v) ? 1.0 : 0.0;
        if (v.type() == typeid(char))   return double(std::any_cast<char>(v));
        if (v.type() == typeid(float))  return double(std::any_cast<float>(v));
        if (v.type() == typeid(double)) return         std::any_cast<double>(v);
        return 0.0;
    };

    auto toInt = [&](const std::any& v) -> int {
        if (!v.has_value()) return 0;
        if (v.type() == typeid(int))    return         std::any_cast<int>(v);
        if (v.type() == typeid(bool))   return std::any_cast<bool>(v) ? 1 : 0;
        if (v.type() == typeid(char))   return int(std::any_cast<char>(v));
        if (v.type() == typeid(float))  return int(std::any_cast<float>(v));
        if (v.type() == typeid(double)) return int(std::any_cast<double>(v));
        return 0;
    };

    bool isFloatOp2 = (lhsVal.type() == typeid(double) || rhsVal.type() == typeid(double)
                    || lhsVal.type() == typeid(float)  || rhsVal.type() == typeid(float));

    std::any resultAny;
    std::shared_ptr<Type> resultType;

    if (op == "+" || op == "-" || op == "*" || op == "/") {
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
    }
    else if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        bool cmp;
        if (isFloatOp2) {
            double l = toDouble(lhsVal), r = toDouble(rhsVal);
            if (op == "<")   cmp = (l < r);
            else if (op == ">")   cmp = (l > r);
            else if (op == "<=")  cmp = (l <= r);
            else if (op == ">=")  cmp = (l >= r);
            else if (op == "==")  cmp = (l == r);
            else /* "!=" */      cmp = (l != r);
        } else {
            int l = toInt(lhsVal), r = toInt(rhsVal);
            if (op == "<")   cmp = (l < r);
            else if (op == ">")   cmp = (l > r);
            else if (op == "<=")  cmp = (l <= r);
            else if (op == ">=")  cmp = (l >= r);
            else if (op == "==")  cmp = (l == r);
            else /* "!=" */      cmp = (l != r);
        }
        resultAny  = cmp;
        resultType = std::make_shared<BoolType>();
    }
    else if (op == "&&" || op == "||") {
        bool lv = toInt(lhsVal) != 0;
        bool rv = toInt(rhsVal) != 0;
        resultAny  = (op == "&&") ? (lv && rv) : (lv || rv);
        resultType = std::make_shared<BoolType>();
    }
    else {
        throw std::runtime_error("unsupported binary operator: " + op);
    }

    return std::make_shared<VarSymbol>(resultType, resultAny);
}


std::shared_ptr<VarSymbol> Execute::unary_operation(std::shared_ptr<VarSymbol> baseSym,
                                                    std::string& op) {
    std::any v = baseSym->value;
    std::any resultAny;
    std::shared_ptr<Type> resultType = baseSym->type;

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
    // Берём текущее значение из baseSym
    std::any oldVal = baseSym->value;
    std::any newVal;

    // Только для int и double поддерживаем ++/--
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

    // Обновляем значение в самой переменной
    baseSym->value = newVal;

    // Возвращаем новый VarSymbol, содержащий прежнее значение (oldVal)
    return std::make_shared<VarSymbol>(baseSym->type, oldVal);
}


bool Execute::can_convert(const std::shared_ptr<Type>& from, const std::shared_ptr<Type>& to) {
    if (from->equals(to)) return true;
    if (dynamic_cast<Arithmetic*>(from.get()) && dynamic_cast<Arithmetic*>(to.get())) return true;
    return false;
}

void Execute::visit(ASTNode&) {
    // Ничего не делаем
}

void Execute::visit(TranslationUnit& unit) {
    for (auto& node : unit.get_nodes()) {
        node->accept(*this);
    }
}

// ——— ДЕКЛАРАТОРЫ ———

void Execute::visit(Declaration::SimpleDeclarator& node) {
}

void Execute::visit(Declaration::PtrDeclarator& node) {
}

void Execute::visit(Declaration::InitDeclarator& node) {
    
}
void Execute::visit(VarDeclaration& node) {
    for (auto& initDecl : node.declarator_list) {
        // -----------------------------
        // 1) Определяем базовый varType
        // -----------------------------
        std::shared_ptr<Type> varType;
        std::any              initValue;

        if (node.type == "auto") {
            // «auto»-объявление: вычисляем тип справа
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
            // Явный тип: либо базовый, либо структурный
            std::shared_ptr<Symbol> typeSym;
            try {
                typeSym = match_symbol(node.type);
            } catch (...) {
                throw std::runtime_error("Symbol or type '" + node.type + "' not found");
            }
            varType = typeSym->type;

            // 1b) Если есть инициализатор, вычисляем значение справа
            if (initDecl->initializer) {
                initDecl->initializer->accept(*this);
                auto rhsSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
                if (!rhsSym) {
                    throw std::runtime_error("initializer is not a VarSymbol");
                }
                initValue = rhsSym->value;
            }
            else {
                // Default-инициализация для «простых» (скалярных) типов
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

        // -------------------------------------------------
        // 2) Если PtrDeclarator, оборачиваем в PointerType:
        // -------------------------------------------------
        if (dynamic_cast<Declaration::PtrDeclarator*>(initDecl->declarator.get())) {
            varType   = std::make_shared<PointerType>(varType);
            initValue = std::any{};
        }

        // -------------------------------------------------
        // 3) Если varType — StructType, создаём новый экземпляр
        // -------------------------------------------------
        if (auto structT = std::dynamic_pointer_cast<StructType>(varType)) {
            // Найдем «шаблонный» StructSymbol для node.type
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

            // Скопировать все VarSymbol-члены с дефолтными значениями
            std::unordered_map<std::string, std::shared_ptr<Symbol>> instance_members;
            for (auto& kv : tmplStruct->members) {
                if (auto fld = std::dynamic_pointer_cast<VarSymbol>(kv.second)) {
                    // Новый VarSymbol того же типа, со значением по умолчанию
                    auto copyVar = std::make_shared<VarSymbol>(fld->type);
                    // Если тип поля — int, float, bool или char, сразу задаем 0/0.0/false/'\0'
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
                    // Для любых других (например, вложенных структур) оставляем std::any{}
                    instance_members[kv.first] = copyVar;
                }
                else if (auto mtd = std::dynamic_pointer_cast<FuncSymbol>(kv.second)) {
                    // Для методов копируем ссылку, их тела будут выполняться при вызове
                    instance_members[kv.first] = mtd;
                }
            }

            // Создать сам StructSymbol-экземпляр
            auto instanceStruct = std::make_shared<StructSymbol>(
                std::static_pointer_cast<StructType>(varType),
                instance_members
            );
            initValue = instanceStruct;
        }

        // -------------------------------------------------
        // 4) Регистрируем глобальную переменную (если её нет)
        // -------------------------------------------------
        const auto& varName = initDecl->declarator->name;
        if (!symbolTable->contains_symbol(varName)) {
            auto newVar = std::make_shared<VarSymbol>(varType);
            symbolTable->push_symbol(varName, newVar);
        }

        // -------------------------------------------------
        // 5) Получаем уже существующий VarSymbol и кладём initValue
        // -------------------------------------------------
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
    // 1) Определяем тип и значение так же, как раньше:
    auto typeName = node.type;
    std::shared_ptr<Type> pType = match_symbol(typeName)->type;
    auto name = node.init_declarator->declarator->name;

    std::any value{};
    if (node.init_declarator->initializer) {
        node.init_declarator->initializer->accept(*this);
        value = std::dynamic_pointer_cast<VarSymbol>(current_value)->value;
    }

    // 2) Не создаём новый VarSymbol, а берём тот, что уже создал Analyzer:
    auto baseSym = symbolTable->match_global(name);
    auto existingParam = std::dynamic_pointer_cast<VarSymbol>(baseSym);
    if (!existingParam) {
        throw std::runtime_error(
            "Internal error: параметр '" + name +
            "' не найден в таблице, хотя Analyzer должен был его зарегистрировать"
        );
    }

    // 3) Записываем туда вычисленное значение:
    existingParam->value = value;
    current_value = existingParam;
}


void Execute::visit(FuncDeclaration& node) {
    // Попытка найти FuncSymbol в текущем скоупе
    std::shared_ptr<Symbol> baseSym;
    bool exists = true;
    try {
        baseSym = symbolTable->match_global(node.declarator->name);
    } catch (...) {
        exists = false;
    }

    // Если символ уже есть (например, метод struct или ранее зарегистрированная функция) — просто выходим
    if (exists) {
        current_value = nullptr;
        return;
    }

    // Иначе — это топ-левел (глобальная) функция. Создаём FuncType и FuncSymbol и пушим его.
    // 1) Определяем возвращаемый тип
    std::shared_ptr<Type> retType;
    if (node.type == "auto") {
        // (для простоты предполагаем, что в вашей программе auto-функций нет, 
        //  или они уже разрешены Analyzer'ом; можно выбросить исключение или трактовать как void)
        retType = std::make_shared<VoidType>();
    } else {
        auto retSym = match_symbol(node.type);
        retType = retSym->type;
        if (node.is_const) {
            retType = std::make_shared<ConstType>(retType);
        }
    }

    // 2) Собираем типы параметров
    std::vector<std::shared_ptr<Type>> argTypes;
    for (auto& p : node.args) {
        auto pSym = match_symbol(p->type);
        auto pType = pSym->type;
        argTypes.push_back(pType);
    }

    // 3) Создаём FuncType и FuncSymbol
    auto fType = std::make_shared<FuncType>(retType, argTypes, node.is_readonly);
    auto fSym = std::make_shared<FuncSymbol>(fType, argTypes, node.is_readonly);
    fSym->declaration = &node;

    // 4) Пушим в глобальный скоуп
    symbolTable->push_symbol(node.declarator->name, fSym);
    current_value = nullptr;
}


void Execute::visit(StructDeclaration& node) {
    // 1) Сперва находим уже существующий StructSymbol, который создал Analyzer
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

    // 2) Извлекаем его поле members (VarSymbol/FuncSymbol для каждого члена)
    //    Эти member_symbols уже были заполнены Analyzer-ом.
    auto member_symbols = structSym->members;

    // 3) Открываем вложенный scope и "заселяем" его всеми VarSymbol / FuncSymbol
    //    из member_symbols, чтобы при рекурсивном вызове visit(...) они находились.
    auto savedScope = symbolTable;
    symbolTable = symbolTable->create_new_table(savedScope);
    for (auto& kv : member_symbols) {
        symbolTable->push_symbol(kv.first, kv.second);
    }

    // 4) Перескакиваем по VarDeclaration внутри тела struct, чтобы проинициализировать
    //    поля (инициализаторами или default‐значениями). Тела методов пропускаем.
    for (auto& m : node.members) {
        if (auto fldDecl = dynamic_cast<VarDeclaration*>(m.get())) {
            // VarDeclaration инициализирует своё поле (initializer'ы),
            // используя тот же visit(VarDeclaration&).
            fldDecl->accept(*this);
        }
        else if (auto mtdDecl = dynamic_cast<FuncDeclaration*>(m.get())) {
            // Ничего не делаем с телом метода на объявлении:
            // тело метода запустится только при вызове (FunctionCallExpression).
            continue;
        }
        else {
            m->accept(*this);
        }
    }

    // 5) После того как все поля структуры инициализированы, возвращаем предыдущий scope
    symbolTable = savedScope;

    // 6) В current_value кладём найденный StructSymbol, 
    //    чтобы, если кто-то сразу после struct-узла посмотрит current_value, он увидел именно его.
    current_value = structSym;
}

void Execute::visit(ArrayDeclaration& node) {
    // 1) Сначала вычисляем размер:
    node.size->accept(*this);
    int sz = std::any_cast<int>(
        std::dynamic_pointer_cast<VarSymbol>(current_value)->value
    );

    // 2) Определяем тип элемента:
    auto elemType = match_symbol(node.type)->type;

    // 3) Заводим вектор длины sz и заполняем “дефолтными” значениями:
    std::vector<std::any> data(sz);
    for (int i = 0; i < sz; ++i) {
        if      (dynamic_cast<IntegerType*>(elemType.get())) data[i] = int(0);
        else if (dynamic_cast<FloatType*>(elemType.get()))   data[i] = double(0.0);
        else if (dynamic_cast<BoolType*>(elemType.get()))    data[i] = false;
        else if (dynamic_cast<CharType*>(elemType.get()))    data[i] = char16_t(0);
        else                                                  data[i] = std::any{};
    }

    // 4) Если у ArrayDeclaration есть список инициализаторов,
    //    проходим по нему и заполняем первые элементы массива:
    //    (замените node.initializers на то, как у вас называется поле списка выражений)
    if (!node.initializer_list.empty()) {
        // сколько элементов приходит в фигурных скобках
        int initCount = static_cast<int>(node.initializer_list.size());
        int limit = std::min(sz, initCount);
        for (int i = 0; i < limit; ++i) {
            node.initializer_list[i]->accept(*this);
            // Ожидаем, что η приемлемые литералы и/или выражения дают VarSymbol
            auto valSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
            if (!valSym) {
                throw std::runtime_error(
                    "array-init: initializer is not a VarSymbol"
                );
            }
            data[i] = valSym->value;
        }
    }

    // 5) Регистрируем или находим VarSymbol для именованного массива:
    std::shared_ptr<VarSymbol> arraySym;
    bool alreadyExists = true;
    try {
        auto sym = symbolTable->match_global(node.name);
        arraySym = std::dynamic_pointer_cast<VarSymbol>(sym);
        if (!arraySym) {
            // Если нашли, но это не VarSymbol → ошибка
            throw std::runtime_error(
                "Symbol '" + node.name + "' is not a variable"
            );
        }
    } catch (...) {
        alreadyExists = false;
    }

    if (!alreadyExists) {
        // 5.1) Создаём тип ArrayType для этой переменной:
        auto arrayType = std::make_shared<ArrayType>(elemType, node.size);
        // 5.2) Создаём VarSymbol с этим типом:
        auto newArrSym = std::make_shared<VarSymbol>(arrayType);
        // 5.3) Регистрируем в текущем symbolTable:
        symbolTable->push_symbol(node.name, newArrSym);
        arraySym = newArrSym;
    }

    // 6) Кладём сформированный вектор data внутрь VarSymbol:
    arraySym->value = std::move(data);

    // 7) Устанавливаем current_value, чтобы следующие выражения видели VarSymbol:
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
    node.base->accept(*this);
    auto baseSym = std::dynamic_pointer_cast<VarSymbol>(current_value);
    if (!baseSym) {
        throw std::runtime_error("prefix: base is not a variable");
    }

    // Префиксный ++
    if (node.op == "++") {
        if (baseSym->value.type() == typeid(int)) {
            int x = std::any_cast<int>(baseSym->value) + 1;
            baseSym->value = x;
            current_value = std::make_shared<VarSymbol>(baseSym->type, x);
        }
        else if (baseSym->value.type() == typeid(double)) {
            double x = std::any_cast<double>(baseSym->value) + 1.0;
            baseSym->value = x;
            current_value = std::make_shared<VarSymbol>(baseSym->type, x);
        }
        else {
            throw std::runtime_error("unsupported operand type for prefix ++");
        }
    }
    // Префиксный --
    else if (node.op == "--") {
        if (baseSym->value.type() == typeid(int)) {
            int x = std::any_cast<int>(baseSym->value) - 1;
            baseSym->value = x;
            current_value = std::make_shared<VarSymbol>(baseSym->type, x);
        }
        else if (baseSym->value.type() == typeid(double)) {
            double x = std::any_cast<double>(baseSym->value) - 1.0;
            baseSym->value = x;
            current_value = std::make_shared<VarSymbol>(baseSym->type, x);
        }
        else {
            throw std::runtime_error("unsupported operand type for prefix --");
        }
    }
    // Все остальные унарные операторы («+», «-», «!» и т.п.)
    else {
        current_value = unary_operation(baseSym, node.op);
    }
}



void Execute::visit(PostfixIncrementExpression& node) {
    // 1) Сначала вычисляем базу
    node.base->accept(*this);
    auto baseSym = std::dynamic_pointer_cast<VarSymbol>(current_value);

    // 2) Оператор всегда "++" для этого узла
    std::string op = "++";

    // 3) Вызываем helper
    auto resultSym = postfix_operation(baseSym, op);

    // 4) Помещаем старое значение в current_value
    current_value = resultSym;
}

void Execute::visit(PostfixDecrementExpression& node) {
    // 1) Сначала вычисляем базу
    node.base->accept(*this);
    auto baseSym = std::dynamic_pointer_cast<VarSymbol>(current_value);

    // 2) Оператор всегда "--" для этого узла
    std::string op = "--";

    // 3) Вызываем helper
    auto resultSym = postfix_operation(baseSym, op);

    // 4) Помещаем старое значение в current_value
    current_value = resultSym;
}



void Execute::visit(FunctionCallExpression& node) {
    // === Шаг 0: если это встроенный вызов print(...) — обрабатываем вручную ===
    if (auto ident = dynamic_cast<IdentifierExpression*>(node.base.get())) {
     if (ident->name == "print") {
            for (size_t i = 0; i < node.args.size(); ++i) {
                node.args[i]->accept(*this);
                auto vsym = std::dynamic_pointer_cast<VarSymbol>(current_value);
                if (!vsym) {
                    std::cout << "<<?>"; 
                } else {
                    if (vsym->value.type() == typeid(std::string)) {
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
                // Берём родительский VarSymbol массива и индекс
                auto parentArr = arrElemSym->parentArray;   // VarSymbol самого массива
                int idx       = arrElemSym->index;         // индекс ячейки

                // Проверим, что parentArr действительно хранит std::vector<std::any>
                auto &vec = std::any_cast<std::vector<std::any>&>(parentArr->value);

                // Теперь посмотрим на тип элемента (elemType)
                auto elemType = std::dynamic_pointer_cast<ArrayType>(parentArr->type)->get_base_type();

                // В зависимости от elemType читаем из std::cin в нужный C++-тип
                if (dynamic_cast<IntegerType*>(elemType.get())) {
                    int v;
                    if (!(std::cin >> v)) {
                        throw std::runtime_error("read(): failed to read an integer from stdin");
                    }
                    vec[idx] = v;
                    arrElemSym->value = v; // обновляем текущее значение элемента
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

                // Возвращаем в current_value тот же ArrayElementSymbol, чтобы дальше его можно было считать
                current_value = arrElemSym;
                return;
            }

            // 3) Если это просто переменная (VarSymbol), а не элемент массива
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

    // 2) Ветка: вызов метода структуры p.move(...)
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

    // 3) Ветка: «свободная» функция f(...)
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

    // 4) Если ни то, ни другое — ошибка
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

void Execute::visit(IdentifierExpression& node) {
    current_value = symbolTable->match_global(node.name);
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


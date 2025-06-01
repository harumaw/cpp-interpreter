#include "executer.hpp"

Execute::Execute()
  : scope(std::make_shared<Scope>(nullptr)),
    return_triggered(false),
    break_triggered(false),
    continue_triggered(false)
{
    // Инициализируем глобальный фрейм (индекс 0)
    call_stack.emplace_back();
}

void Execute::execute(TranslationUnit& unit){
      for (auto& node : unit.get_nodes()) {
        if (auto fdecl = dynamic_cast<FuncDeclaration*>(node.get())) {
            functions[fdecl->declarator->name] = fdecl;
        }
    }
    if (functions.find("main") == functions.end()) {
        throw std::runtime_error("function main not found");
    }

    auto main_identifier = std::make_shared<IdentifierExpression>("main");
    FunctionCallExpression main_call(main_identifier, std::vector<std::shared_ptr<Expression>>{});


    visit(main_call);
    
}

void Execute::visit(ASTNode& node){}

void Execute::visit(TranslationUnit& unit) {
    for (auto& decl : unit.get_nodes()) {
        decl->accept(*this);
    }
}


void Execute::visit(Declaration::PtrDeclarator&)    { }
void Execute::visit(Declaration::SimpleDeclarator&) { }
void Execute::visit(Declaration::InitDeclarator&)   { }


void Execute::visit(VarDeclaration& node) {
    for (auto& decl : node.declarator_list) {
        std::string name = decl->declarator->name;
        call_stack.back().vars[name] = 0;
        if (decl->initializer) {
            decl->initializer->accept(*this);
            call_stack.back().vars[name] = current_value;
        }
    }
}


void Execute::visit(ParameterDeclaration&) { }


void Execute::visit(FuncDeclaration& node) {
    node.body->accept(*this);

    if (!return_triggered) {
        return_value = 0;
        return_triggered = true;
    }
}

void Execute::visit(StructDeclaration& /*node*/) { }
void Execute::visit(ArrayDeclaration& /*node*/)  { }
void Execute::visit(NameSpaceDeclaration& /*node*/) { }

void Execute::visit(CompoundStatement& node) {
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
        if (return_triggered || break_triggered || continue_triggered) {
            break;
        }
    }
    // Сбрасываем break/continue после выхода из блока
    break_triggered = false;
    continue_triggered = false;
}

void Execute::visit(DeclarationStatement& node) {
    node.declaration->accept(*this);
}

void Execute::visit(ExpressionStatement& node) {
    node.expression->accept(*this);
}

void Execute::visit(ConditionalStatement& node) {
    // if (cond) then { ... } else { ... }
    node.if_branch.first->accept(*this);
    bool cond_val = std::any_cast<bool>(current_value);
    if (cond_val) {
        node.if_branch.second->accept(*this);
    }
    else if (node.else_branch) {
        node.else_branch->accept(*this);
    }
}

void Execute::visit(WhileStatement& node) {
    while (true) {
        node.condition->accept(*this);
        if (!std::any_cast<bool>(current_value)) break;
        node.statement->accept(*this);
        if (break_triggered) {
            break_triggered = false;
            break;
        }
        if (continue_triggered) {
            continue_triggered = false;
            continue;
        }
    }
}

void Execute::visit(ForStatement& node) {
    if (node.initialization)
        node.initialization->accept(*this);
    while (true) {
        if (node.condition) {
            node.condition->accept(*this);
            if (!std::any_cast<bool>(current_value)) break;
        }
        node.body->accept(*this);
        if (break_triggered) {
            break_triggered = false;
            break;
        }
        if (continue_triggered) {
            continue_triggered = false;
        }
        if (node.increment)
            node.increment->accept(*this);
    }
}

void Execute::visit(ReturnStatement& node) {
    if (node.expression) {
        node.expression->accept(*this);
        return_value = current_value;
    }
    else {
        return_value = 0; // default
    }
    return_triggered = true;
}

void Execute::visit(BreakStatement& /*node*/) {
    break_triggered = true;
}

void Execute::visit(ContinueStatement& /*node*/) {
    continue_triggered = true;
}

void Execute::visit(StructMemberAccessExpression& /*node*/) {
    // Пока не поддерживается рантайм-доступ к полям struct
}

void Execute::visit(DoWhileStatement& node) {
    do {
        node.statement->accept(*this);
        if (break_triggered) {
            break_triggered = false;
            break;
        }
        if (continue_triggered) {
            continue_triggered = false;
        }
        node.condition->accept(*this);
    } while (std::any_cast<bool>(current_value));
}

void Execute::visit(BinaryOperation& node) {
    // 1) Считаем значения левого и правого операндов:
    node.lhs->accept(*this);
    std::any lhs_value = current_value;
    node.rhs->accept(*this);
    std::any rhs_value = current_value;

    const std::string& op = node.op;
    const std::type_info& lt = lhs_value.type();
    const std::type_info& rt = rhs_value.type();
    std::any result;

    // ------------------------
    // АРИФМЕТИЧЕСКИЕ ОПЕРАЦИИ
    // ------------------------
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        if (lt == typeid(double) || rt == typeid(double)) {
            double a = (lt == typeid(double))
                           ? std::any_cast<double>(lhs_value)
                           : (lt == typeid(int)
                                  ? static_cast<double>(std::any_cast<int>(lhs_value))
                                  : (lt == typeid(char)
                                         ? static_cast<double>(std::any_cast<char>(lhs_value))
                                         : (lt == typeid(bool)
                                                ? static_cast<double>(std::any_cast<bool>(lhs_value))
                                                : 0.0)));
            double b = (rt == typeid(double))
                           ? std::any_cast<double>(rhs_value)
                           : (rt == typeid(int)
                                  ? static_cast<double>(std::any_cast<int>(rhs_value))
                                  : (rt == typeid(char)
                                         ? static_cast<double>(std::any_cast<char>(rhs_value))
                                         : (rt == typeid(bool)
                                                ? static_cast<double>(std::any_cast<bool>(rhs_value))
                                                : 0.0)));
            if (op == "+")         result = a + b;
            else if (op == "-")    result = a - b;
            else if (op == "*")    result = a * b;
            else /* op == "/" */   result = a / b;
        }
        else if (lt == typeid(int) && rt == typeid(int)) {
            int a = std::any_cast<int>(lhs_value);
            int b = std::any_cast<int>(rhs_value);
            if (op == "+")       result = a + b;
            else if (op == "-")  result = a - b;
            else if (op == "*")  result = a * b;
            else /* op == "/" */ result = a / b;
        }
        else if (lt == typeid(char) && rt == typeid(char)) {
            char a = std::any_cast<char>(lhs_value);
            char b = std::any_cast<char>(rhs_value);
            char r = 0;
            if (op == "+")       r = static_cast<char>(a + b);
            else if (op == "-")  r = static_cast<char>(a - b);
            else if (op == "*")  r = static_cast<char>(a * b);
            else /* op == "/" */ r = static_cast<char>(a / b);
            result = r;
        }
        else if (lt == typeid(bool) && rt == typeid(bool)) {
            int a = std::any_cast<bool>(lhs_value) ? 1 : 0;
            int b = std::any_cast<bool>(rhs_value) ? 1 : 0;
            int r = 0;
            if (op == "+")       r = a + b;
            else if (op == "-")  r = a - b;
            else if (op == "*")  r = a * b;
            else /* op == "/" */ r = (b != 0 ? a / b : 0);
            result = r;
        }
        else if ((lt == typeid(int) && rt == typeid(char)) ||
                 (lt == typeid(char) && rt == typeid(int)) ||
                 (lt == typeid(int) && rt == typeid(bool)) ||
                 (lt == typeid(bool) && rt == typeid(int)) ||
                 (lt == typeid(char) && rt == typeid(bool)) ||
                 (lt == typeid(bool) && rt == typeid(char))) {
            int a = 0, b = 0;
            if      (lt == typeid(int))  a = std::any_cast<int>(lhs_value);
            else if (lt == typeid(char)) a = static_cast<int>(std::any_cast<char>(lhs_value));
            else                          a = std::any_cast<bool>(lhs_value) ? 1 : 0;

            if      (rt == typeid(int))  b = std::any_cast<int>(rhs_value);
            else if (rt == typeid(char)) b = static_cast<int>(std::any_cast<char>(rhs_value));
            else                          b = std::any_cast<bool>(rhs_value) ? 1 : 0;

            int r = 0;
            if (op == "+")       r = a + b;
            else if (op == "-")  r = a - b;
            else if (op == "*")  r = a * b;
            else /* op == "/" */ r = (b != 0 ? a / b : 0);
            result = r;
        }
        else {
            throw std::runtime_error("Unsupported operand types for operator '" + op + "'");
        }
    }
    // ------------------------
    // ОПЕРАЦИИ СРАВНЕНИЯ
    // ------------------------
    else if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        bool cmp = false;
        if (lt == typeid(double) || rt == typeid(double)) {
            double a = (lt == typeid(double))
                           ? std::any_cast<double>(lhs_value)
                           : (lt == typeid(int)
                                  ? static_cast<double>(std::any_cast<int>(lhs_value))
                                  : (lt == typeid(char)
                                         ? static_cast<double>(std::any_cast<char>(lhs_value))
                                         : (lt == typeid(bool)
                                                ? static_cast<double>(std::any_cast<bool>(lhs_value))
                                                : 0.0)));
            double b = (rt == typeid(double))
                           ? std::any_cast<double>(rhs_value)
                           : (rt == typeid(int)
                                  ? static_cast<double>(std::any_cast<int>(rhs_value))
                                  : (rt == typeid(char)
                                         ? static_cast<double>(std::any_cast<char>(rhs_value))
                                         : (rt == typeid(bool)
                                                ? static_cast<double>(std::any_cast<bool>(rhs_value))
                                                : 0.0)));
            if (op == "<")       cmp = (a < b);
            else if (op == ">")  cmp = (a > b);
            else if (op == "<=") cmp = (a <= b);
            else if (op == ">=") cmp = (a >= b);
            else if (op == "==") cmp = (a == b);
            else                  cmp = (a != b);
        }
        else if (lt == typeid(int) && rt == typeid(int)) {
            int a = std::any_cast<int>(lhs_value);
            int b = std::any_cast<int>(rhs_value);
            if (op == "<")       cmp = (a < b);
            else if (op == ">")  cmp = (a > b);
            else if (op == "<=") cmp = (a <= b);
            else if (op == ">=") cmp = (a >= b);
            else if (op == "==") cmp = (a == b);
            else                  cmp = (a != b);
        }
        else if (lt == typeid(char) && rt == typeid(char)) {
            char a = std::any_cast<char>(lhs_value);
            char b = std::any_cast<char>(rhs_value);
            if (op == "<")       cmp = (a < b);
            else if (op == ">")  cmp = (a > b);
            else if (op == "<=") cmp = (a <= b);
            else if (op == ">=") cmp = (a >= b);
            else if (op == "==") cmp = (a == b);
            else                  cmp = (a != b);
        }
        else if (lt == typeid(bool) && rt == typeid(bool)) {
            bool a = std::any_cast<bool>(lhs_value);
            bool b = std::any_cast<bool>(rhs_value);
            if (op == "<")       cmp = (a < b);
            else if (op == ">")  cmp = (a > b);
            else if (op == "<=") cmp = (a <= b);
            else if (op == ">=") cmp = (a >= b);
            else if (op == "==") cmp = (a == b);
            else                  cmp = (a != b);
        }
        else if ((lt == typeid(int) && rt == typeid(char)) ||
                 (lt == typeid(char) && rt == typeid(int)) ||
                 (lt == typeid(int) && rt == typeid(bool)) ||
                 (lt == typeid(bool) && rt == typeid(int)) ||
                 (lt == typeid(char) && rt == typeid(bool)) ||
                 (lt == typeid(bool) && rt == typeid(char))) {
            int a = 0, b = 0;
            if      (lt == typeid(int))  a = std::any_cast<int>(lhs_value);
            else if (lt == typeid(char)) a = static_cast<int>(std::any_cast<char>(lhs_value));
            else                          a = std::any_cast<bool>(lhs_value) ? 1 : 0;

            if      (rt == typeid(int))  b = std::any_cast<int>(rhs_value);
            else if (rt == typeid(char)) b = static_cast<int>(std::any_cast<char>(rhs_value));
            else                          b = std::any_cast<bool>(rhs_value) ? 1 : 0;

            if (op == "<")       cmp = (a < b);
            else if (op == ">")  cmp = (a > b);
            else if (op == "<=") cmp = (a <= b);
            else if (op == ">=") cmp = (a >= b);
            else if (op == "==") cmp = (a == b);
            else                  cmp = (a != b);
        }
        else {
            throw std::runtime_error("Unsupported operand types for comparison '" + op + "'");
        }
        result = cmp;
    }
    // ------------------------
    // ЛОГИЧЕСКИЕ ОПЕРАЦИИ
    // ------------------------
    else if (op == "&&" || op == "||") {
        // Оба операнда должны приводиться к bool:
        bool a = false, b = false;
        if      (lt == typeid(bool)) a = std::any_cast<bool>(lhs_value);
        else if (lt == typeid(int))  a = (std::any_cast<int>(lhs_value) != 0);
        else if (lt == typeid(char)) a = (std::any_cast<char>(lhs_value) != 0);
        else if (lt == typeid(double)) a = (std::any_cast<double>(lhs_value) != 0.0);
        else                          throw std::runtime_error("Unsupported type for logical operand");

        if      (rt == typeid(bool)) b = std::any_cast<bool>(rhs_value);
        else if (rt == typeid(int))  b = (std::any_cast<int>(rhs_value) != 0);
        else if (rt == typeid(char)) b = (std::any_cast<char>(rhs_value) != 0);
        else if (rt == typeid(double)) b = (std::any_cast<double>(rhs_value) != 0.0);
        else                          throw std::runtime_error("Unsupported type for logical operand");

        bool r = false;
        if (op == "&&") r = (a && b);
        else             r = (a || b);
        result = r;
    }
    // ------------------------
    // НЕПОДДЕРЖИВАЕМЫЙ ОПЕРАТОР
    // ------------------------
    else {
        throw std::runtime_error("Unsupported binary operator: " + op);
    }

    // Сохраняем в current_value:
    current_value = std::move(result);
}

void Execute::visit(IntLiteral& node) {
    // node.value — целочисленное (int)
    current_value = static_cast<int>(node.value);
}

void Execute::visit(FloatLiteral& node) {
    // node.value — число с плавающей точкой (double)
    current_value = static_cast<double>(node.value);
}

void Execute::visit(CharLiteral& node) {
    // node.value — встроенный integer, приводим к char
    current_value = static_cast<char>(node.value);
}

void Execute::visit(StringLiteral& node) {
    // node.value — std::string
    current_value = std::string(node.value);
}

void Execute::visit(BoolLiteral& node) {
    // node.value — bool
    current_value = static_cast<bool>(node.value);
}



// executer.cpp

void Execute::visit(IdentifierExpression& node) {
    // Попытаемся найти символ (VarSymbol или константу) в текущей таблице:
    std::shared_ptr<Symbol> sym;
    try {
        sym = scope->match_global(node.name);
    } catch (const std::runtime_error&) {
        throw std::runtime_error("undefined variable: " + node.name);
    }

}


void Execute::visit(PrefixExpression& node) {
    // node.op может быть "-", "+" или "!" (логическое НЕ)
    node.base->accept(*this);

    // Разрешим три случая:
    if (node.op == "-") {
        if (current_value.type() == typeid(int)) {
            current_value = -std::any_cast<int>(current_value);
        } else if (current_value.type() == typeid(double)) {
            current_value = -std::any_cast<double>(current_value);
        } else {
            throw std::runtime_error("invalid type for unary '-'");
        }
    }
    else if (node.op == "+") {
        // «плюс» просто возвращает то же значение
        // (но проверим, что это число)
        if (current_value.type() == typeid(int)
         || current_value.type() == typeid(double)) {
            // оставляем без изменений
        } else {
            throw std::runtime_error("invalid type for unary '+'");
        }
    }
    else if (node.op == "!") {
        if (current_value.type() == typeid(bool)) {
            current_value = ! std::any_cast<bool>(current_value);
        } else {
            throw std::runtime_error("invalid type for logical '!'");
        }
    }
    else {
        throw std::runtime_error("unsupported unary operator: " + node.op);
    }
}

////////////////////////////////////////
// Постфиксная инкремента/декремента
////////////////////////////////////////

void Execute::visit(PostfixIncrementExpression& node) {
    // node.base—должно быть lvalue (IdentifierExpression или SubscriptExpression)
    // сохраним старое значение, затем увеличим в месте хранения, 
    // а в current_value положим старое (постфикс).
    // Для простоты реализуем только для IdentifierExpression:
    auto ident = dynamic_cast<IdentifierExpression*>(node.base.get());
    if (!ident) {
        throw std::runtime_error("postfix '++' base must be identifier");
    }
    // найдём VarSymbol
    auto sym = scope->match_global(ident->name);
    auto varSym = std::dynamic_pointer_cast<VarSymbol>(sym);
    if (!varSym) {
        throw std::runtime_error("identifier is not a variable in postfix '++'");
    }

    // старое:
    std::any old_val = varSym->value;
    // инкрементируем на месте:
    if (old_val.type() == typeid(int)) {
        int tmp = std::any_cast<int>(old_val) + 1;
        varSym->value = tmp;
    }
    else if (old_val.type() == typeid(double)) {
        double tmp = std::any_cast<double>(old_val) + 1.0;
        varSym->value = tmp;
    }
    else if (old_val.type() == typeid(char)) {
        char tmp = (char)(std::any_cast<char>(old_val) + 1);
        varSym->value = tmp;
    }
    else {
        throw std::runtime_error("invalid type for postfix '++'");
    }

    // вернём старое:
    current_value = old_val;
}

void Execute::visit(PostfixDecrementExpression& node) {
    auto ident = dynamic_cast<IdentifierExpression*>(node.base.get());
    if (!ident) {
        throw std::runtime_error("postfix '--' base must be identifier");
    }
    auto sym = scope->match_global(ident->name);
    auto varSym = std::dynamic_pointer_cast<VarSymbol>(sym);
    if (!varSym) {
        throw std::runtime_error("identifier is not a variable in postfix '--'");
    }

    std::any old_val = varSym->value;
    if (old_val.type() == typeid(int)) {
        int tmp = std::any_cast<int>(old_val) - 1;
        varSym->value = tmp;
    }
    else if (old_val.type() == typeid(double)) {
        double tmp = std::any_cast<double>(old_val) - 1.0;
        varSym->value = tmp;
    }
    else if (old_val.type() == typeid(char)) {
        char tmp = (char)(std::any_cast<char>(old_val) - 1);
        varSym->value = tmp;
    }
    else {
        throw std::runtime_error("invalid type for postfix '--'");
    }

    current_value = old_val;
}

void Execute::visit(ParenthesizedExpression& node) {
    node.expression->accept(*this);
}

void Execute::visit(TernaryExpression& node) {
    node.condition->accept(*this);
    bool cond = std::any_cast<bool>(current_value);
    if (cond) {
        node.true_expr->accept(*this);
    } else {
        node.false_expr->accept(*this);
    }
}
void Execute::visit(FunctionCallExpression& node) {
    // 1) Вычисляем «фактические» аргументы (их значения и «прототипные» типы, если нужно)
    std::vector<std::any> arg_values;
    std::vector<std::shared_ptr<Type>> arg_types;

    for (auto& a : node.args) {
        a->accept(*this);
        // current_value теперь содержит std::any с вычисленным значением аргумента
        arg_values.push_back(current_value);

        // Запомним также грубо «тип» аргумента, чтобы, при желании, можно было проверить сигнатуру.
        // Здесь просто создаём новый объект нужного Fundamental типа:
        if (current_value.type() == typeid(int)) {
            arg_types.push_back(std::make_shared<IntegerType>());
        }
        else if (current_value.type() == typeid(double)) {
            arg_types.push_back(std::make_shared<FloatType>());
        }
        else if (current_value.type() == typeid(char)) {
            arg_types.push_back(std::make_shared<CharType>());
        }
        else if (current_value.type() == typeid(bool)) {
            arg_types.push_back(std::make_shared<BoolType>());
        }
        else if (current_value.type() == typeid(std::string)) {
            arg_types.push_back(std::make_shared<StringType>());
        }
        else {
            // Для всех остальных (например, struct-переменные или указатели) — оставляем nullptr
            arg_types.push_back(nullptr);
        }
    }

    // 2) Извлекаем имя функции из node.base (ожидаем, что base — это IdentifierExpression)
    auto ident = dynamic_cast<IdentifierExpression*>(node.base.get());
    if (!ident) {
        throw std::runtime_error("FunctionCallExpression: base is not an identifier");
    }
    const std::string fun_name = ident->name;

    // 3) Ищем определение функции в function_defs
    auto it = function_defs.find(fun_name);
    if (it == function_defs.end()) {
        throw std::runtime_error("undefined function: " + fun_name);
    }
    FuncDeclaration* fdecl = it->second;

    // 4) Простая проверка числа параметров (перегрузки не поддерживаются)
    if (fdecl->args.size() != arg_values.size()) {
        throw std::runtime_error(
            "argument count mismatch in call to '" + fun_name + "'"
        );
    }

    // 5) Создаём новый вложенный scope для тела функции
    auto saved_scope = current_scope;
    current_scope = current_scope->create_new_table(saved_scope);

    // 6) Заселяем параметры–аргументы в новую область видимости
    for (size_t i = 0; i < fdecl->args.size(); ++i) {
        // Имя i-го формального параметра:
        const std::string& pname = fdecl->args[i]->init_declarator->declarator->name;

        // Вычисляем и «воссоздаём» тип параметра (должно совпадать с семантическим анализом)
        std::shared_ptr<Type> ptype;
        const std::string& ptype_name = fdecl->args[i]->type;
        if (ptype_name == "int") {
            ptype = std::make_shared<IntegerType>();
        }
        else if (ptype_name == "float") {
            ptype = std::make_shared<FloatType>();
        }
        else if (ptype_name == "char") {
            ptype = std::make_shared<CharType>();
        }
        else if (ptype_name == "bool") {
            ptype = std::make_shared<BoolType>();
        }
        else if (ptype_name == "string") {
            ptype = std::make_shared<StringType>();
        }
        else {
            // может быть struct-тип
            auto sym = current_scope->match_global(ptype_name);
            auto ss = std::dynamic_pointer_cast<StructSymbol>(sym);
            if (!ss) {
                throw std::runtime_error("unknown param type in function call: " + ptype_name);
            }
            ptype = ss->type;
        }

        // Создаём VarSymbol для параметра и кладём туда значение
        auto vsym = std::make_shared<VarSymbol>(ptype);
        vsym->value = arg_values[i];
        current_scope->push_symbol(pname, vsym);
    }

    // 7) Выполняем тело функции
    returned = false;
    return_value.reset();
    fdecl->body->accept(*this);

    // 8) Если внутри тела встретился return, достаём return_value, иначе — 0
    std::any result;
    if (returned) {
        result = return_value;
    } else {
        // Если функция не вернула значение явно, считаем, что она возвращает int 0
        result = int(0);
    }

    // 9) Восстанавливаем предыдущую область видимости
    current_scope = saved_scope;

    // 10) Кладём результат вызова в current_value
    current_value = result;
}

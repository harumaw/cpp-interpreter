#include "analyzer.hpp"
#include "token.hpp"
#include "semantic_exception.hpp"

int getTypeRank(const Type& type) {
    if (dynamic_cast<const FloatType*>(&type))    return  3;
    if (dynamic_cast<const IntegerType*>(&type))  return  2;
    if (dynamic_cast<const CharType*>(&type))     return  1;
    if (dynamic_cast<const BoolType*>(&type))     return 0;
    return -2;
}

std::shared_ptr<Type> compareRank(const std::shared_ptr<Type>& lhs, const std::shared_ptr<Type>& rhs) {
    int rl = getTypeRank(*lhs);
    int rr = getTypeRank(*rhs);
    int maxr = std::max(rl, rr);

    auto intType = Analyzer::default_types.at("int");
    int intRank = getTypeRank(*intType);
    if (maxr < intRank) {
        return intType;
    }

    if (rl >= rr) {
        return lhs;
    } else {
        return rhs;
    }
}


bool canConvert(const std::shared_ptr<Type>& from, const std::shared_ptr<Type>& to) {
    auto strip = [](std::shared_ptr<Type> t){
        if (auto cp = dynamic_cast<ConstType*>(t.get()))
            return cp->get_base();
        return t;
    };
    auto f = strip(from);
    auto tt = strip(to);

    if (f->equals(tt)) return true;
    // арифметика тоже может конвертироваться
    if (dynamic_cast<Arithmetic*>(f.get()) && dynamic_cast<Arithmetic*>(tt.get()))
        return true;
    return false;
}

std::unordered_map<std::string, std::shared_ptr<Type>> Analyzer::default_types = {
    {"int",    std::make_shared<IntegerType>()},
    {"float",  std::make_shared<FloatType>()},
    {"char",   std::make_shared<CharType>()},
    {"bool",   std::make_shared<BoolType>()},
    {"void", std::make_shared<VoidType>()}
};

Analyzer::Analyzer()
  : scope(std::make_shared<Scope>(nullptr))
  , current_type(nullptr)
{}

void Analyzer::analyze(TranslationUnit& unit) {
    scope = std::make_shared<Scope>(nullptr);
    errors.clear();

    for (auto& node : unit.get_nodes()) {
        try {
            visit(*node);
        } catch (const SemanticException& e) {
            errors.push_back(e.what());
        }
    }
}

void Analyzer::visit(TranslationUnit& unit) {
    VISIT_BODY_BEGIN
    for (auto& node : unit.get_nodes()) {
        visit(*node);
    }
    VISIT_BODY_END
}

void Analyzer::visit(ASTNode& node) {
    VISIT_BODY_BEGIN
    node.accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(Declaration::SimpleDeclarator& node) {
    VISIT_BODY_BEGIN
    VISIT_BODY_END
}

void Analyzer::visit(Declaration::PtrDeclarator& node) {
    VISIT_BODY_BEGIN
    current_type = std::make_shared<PointerType>(current_type);
    VISIT_BODY_END
}

void Analyzer::visit(Declaration::InitDeclarator& node) {
    VISIT_BODY_BEGIN
    node.declarator->accept(*this);
    if (node.initializer)
        node.initializer->accept(*this);
    VISIT_BODY_END
}


void Analyzer::visit(VarDeclaration& node) {
    VISIT_BODY_BEGIN

    std::shared_ptr<Type> base_t;
    bool    deduce_auto = false;

    if (node.type == "auto") {
        deduce_auto = true;

        // 1) проверяем, что есть хотя бы один инициализатор
        if (node.declarator_list.empty()) {
            throw SemanticException("auto-declaration requires at least one declarator");
        }

        // Вариант СТРОГИЙ: разрешаем только ровно один declarator с инициализатором
        if (node.declarator_list.size() != 1 ||
            !node.declarator_list.front()->initializer) 
        {
            throw SemanticException("auto-declaration must have exactly one initializer");
        }

        // 2) Дедуцируем тип из единственного initializer
        auto& the_decl = node.declarator_list.front();
        current_type = nullptr;
        the_decl->initializer->accept(*this);
        if (!current_type) {
            throw SemanticException("cannot deduce type for auto");
        }
        // Убираем верхний const, если он есть
        if (auto cp = dynamic_cast<ConstType*>(current_type.get())) {
            base_t = cp->get_base();
        } else {
            base_t = current_type;
        }
    }
    else {
        // Обычная ветка: тип задаётся явно
        base_t = get_type(node.type);
        if (node.is_const) {
            base_t = std::make_shared<ConstType>(base_t);
        }
    }

    // 3) Регистрируем все переменные из списка
    for (auto& decl : node.declarator_list) {
        const std::string& name = decl->declarator->name;
        if (scope->contains_symbol(name)) {
            throw SemanticException("variable already declared: " + name);
        }
        scope->push_symbol(name, std::make_shared<VarSymbol>(base_t));

        // Если у него есть initializer, проверяем совместимость
        if (decl->initializer) {
            if (!deduce_auto) {
                // Если не auto, то вычисляем RHS здесь
                decl->initializer->accept(*this);
            }
            // Сейчас current_type — тип выражения справа
            if (!canConvert(current_type, base_t)) {
                throw SemanticException("cannot initialize variable '" + name + "' with given type");
            }
        }
    }

    VISIT_BODY_END
}



void Analyzer::visit(ParameterDeclaration& node) {
    VISIT_BODY_BEGIN
    auto base_t = get_type(node.type);
    const auto& name = node.init_declarator->declarator->name;
    if (scope->contains_symbol(name)) {
        throw SemanticException("parameter already declared: " + name);
    }
    scope->push_symbol(name, std::make_shared<VarSymbol>(base_t));

    if (node.init_declarator->initializer) {
        node.init_declarator->initializer->accept(*this);
        if (!canConvert(current_type, base_t)) {
            throw SemanticException("cannot initialize parameter '" + name + "' with given type");
        }
    }
    VISIT_BODY_END
}


void Analyzer::visit(FuncDeclaration& node) {
    VISIT_BODY_BEGIN

    std::shared_ptr<Type> ret_t;

    if (node.type == "auto") {
        // ======== БЛОК СТРОГОЙ ДЕДУКЦИИ «на месте» ========
        // Сохраним текущее состояние

        auto saved_scope = scope;
        bool saved_flag = is_deducing_return;
        auto saved_deduced = deduced_return_type;

        // Включаем режим дедукции
        is_deducing_return = true;
        deduced_return_type = nullptr;

        // Создаём временный скоуп для тела функции
        scope = scope->create_new_table(saved_scope);

        // Регистрируем параметры, чтобы их можно было использовать в return-выражениях
        std::vector<std::shared_ptr<Type>> arg_ts_for_deduce;
        for (auto& p : node.args) {
            auto pt = get_type(p->type);
            arg_ts_for_deduce.push_back(pt);
        }
        for (size_t i = 0; i < node.args.size(); ++i) {
            const auto& pname = node.args[i]->init_declarator->declarator->name;
            scope->push_symbol(pname, std::make_shared<VarSymbol>(arg_ts_for_deduce[i]));
        }

        // Обход тела в режиме дедукции
        node.body->accept(*this);

        // Если ни одного return с expr не было, считаем void
        if (!deduced_return_type) {
            ret_t = Analyzer::default_types.at("void");
        } else {
            ret_t = deduced_return_type;
        }

        // Восстанавливаем старое состояние
        scope = saved_scope;
        is_deducing_return = saved_flag;
        deduced_return_type = saved_deduced;
        // ======== КОНЕЦ БЛОКА ДЕДУКЦИИ ========
    }
    else {
        // Обычная ветка: явно указанный тип
        ret_t = get_type(node.type);
        if (node.is_const) {
            ret_t = std::make_shared<ConstType>(ret_t);
        }
    }

    // Регистрируем функцию в текущем скоупе (с уже известным ret_t)
    std::vector<std::shared_ptr<Type>> arg_ts;
    for (auto& p : node.args) {
        arg_ts.push_back(get_type(p->type));
    }

    const auto& fname = node.declarator->name;
    if (scope->contains_symbol(fname)) {
        throw SemanticException("function already declared: " + fname);
    }

    auto signature = std::make_shared<FuncType>(ret_t, arg_ts, node.is_readonly);
    scope->push_symbol(
        fname,
        std::make_shared<FuncSymbol>(signature, arg_ts, node.is_readonly)
    );

    // ======== Второй проход: валидация тела с известным ret_t ========
    return_type_stack.push_back(ret_t);
    auto saved_scope2 = scope;
    scope = scope->create_new_table(saved_scope2);

    // Регистрируем параметры как локальные переменные
    for (size_t i = 0; i < node.args.size(); ++i) {
        const auto& pname = node.args[i]->init_declarator->declarator->name;
        scope->push_symbol(pname, std::make_shared<VarSymbol>(arg_ts[i]));

        // Если у параметра есть initializer, проверяем canConvert
        if (node.args[i]->init_declarator->initializer) {
            node.args[i]->init_declarator->initializer->accept(*this);
            if (!canConvert(current_type, arg_ts[i])) {
                throw SemanticException(
                    "cannot initialize parameter '" + pname + "' with given type"
                );
            }
        }
    }

    // Собственно, полный обход тела с проверками return/других операторов
    node.body->accept(*this);

    // Восстанавливаем скоуп и удаляем тип из стека
    scope = saved_scope2;
    return_type_stack.pop_back();
    // ======== Конец второго прохода ========

    VISIT_BODY_END
}

void Analyzer::visit(StructDeclaration& node) {
    VISIT_BODY_BEGIN

    // 1) Проверка на повторное объявление именно в этой области видимости
    if (scope->contains_symbol(node.name)) {
        throw SemanticException("struct already declared: " + node.name);
    }

    // 2) Собираем описание полей и методов в локальные map-ы
    std::unordered_map<std::string, std::shared_ptr<Type>> data_members;
    std::unordered_map<std::string, std::shared_ptr<FuncType>> methods;

    for (auto& m : node.members) {
        if (auto fld = dynamic_cast<VarDeclaration*>(m.get())) {
            // анализ поля, чтобы в current_type лежал его тип
            fld->accept(*this);
            const auto& fname = fld->declarator_list[0]->declarator->name;
            if (data_members.count(fname)) {
                throw SemanticException("duplicate struct member: " 
                                         + fname + " in struct " + node.name);
            }
            data_members[fname] = current_type;

        } else if (auto mtd = dynamic_cast<FuncDeclaration*>(m.get())) {
            // строим FuncType для метода
            auto m_ret = get_type(mtd->type);
            if (mtd->is_const) {
                m_ret = std::make_shared<ConstType>(m_ret);
            }

            std::vector<std::shared_ptr<Type>> m_args;
            for (auto& p : mtd->args) {
                auto at = get_type(p->type);
                p->init_declarator->declarator->accept(*this);
                m_args.push_back(current_type);
            }

            auto m_ft = std::make_shared<FuncType>(m_ret, m_args, mtd->is_readonly);
            const auto& mname = mtd->declarator->name;
            if (methods.count(mname)) {
                throw SemanticException("duplicate struct method: " 
                                         + mname + " in struct " + node.name);
            }
            methods[mname] = m_ft;

        } else {
            throw SemanticException("invalid struct member declaration in struct " 
                                     + node.name);
        }
    }

    // 3a) Создаём тип записи
    auto struct_type = std::make_shared<StructType>(data_members, methods);

    // 3b) Превращаем поля и методы в символы
    std::unordered_map<std::string, std::shared_ptr<Symbol>> member_symbols;
    for (auto& [name, ty] : data_members) {
        member_symbols[name] = std::make_shared<VarSymbol>(ty);
    }
    for (auto& [name, ft] : methods) {
        member_symbols[name] = std::make_shared<FuncSymbol>(
            ft,
            ft->get_args(),
            ft->is_method_const()
        );
    }

    // 3c) Регистрируем RecordSymbol
    auto struct_sym = std::make_shared<StructSymbol>(
        struct_type,
        std::move(member_symbols)
    );
    scope->push_symbol(node.name, struct_sym);

    VISIT_BODY_END
}



void Analyzer::visit(ArrayDeclaration& node) {
    VISIT_BODY_BEGIN

    node.size->accept(*this);
    if (!dynamic_cast<Integral*>(current_type.get())) {
        throw SemanticException("array size must be integer");
    }

    // базовый тип массива
    auto base_t = get_type(node.type);
    auto arr_t  = std::make_shared<ArrayType>(base_t, node.size);

    // проверяем, нет ли уже в текущем скоупе символа с таким именем
    if (scope->contains_symbol(node.name)) {
        throw SemanticException("variable already declared: " + node.name);
    }

    // регистрируем переменную–массив как символ
    scope->push_symbol(
        node.name,
        std::make_shared<VarSymbol>(arr_t)    // или ArraySymbol, если вы его завели
    );

    // результирующий тип этого выражения — массив
    current_type = arr_t;

    VISIT_BODY_END
}


void Analyzer::visit(CompoundStatement& node) {
    VISIT_BODY_BEGIN

    auto saved = scope;
    scope = scope->create_new_table(saved);

    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }

    scope = saved;

    VISIT_BODY_END
}

void Analyzer::visit(NameSpaceDeclaration& node) {
    VISIT_BODY_BEGIN

    auto saved = scope;
    scope = scope->create_new_table(saved);

    
    for (auto& decl : node.declarations) {
        decl->accept(*this);
    }

    // Возвращаемся в внешний скоуп
    scope = saved;

    VISIT_BODY_END
}

void Analyzer::visit(DeclarationStatement& node) {
    VISIT_BODY_BEGIN
    node.declaration->accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(ExpressionStatement& node) {
    VISIT_BODY_BEGIN
    node.expression->accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(ConditionalStatement& node) {
    VISIT_BODY_BEGIN

    node.if_branch.first->accept(*this);
    if (!dynamic_cast<BoolType*>(current_type.get()))
        throw SemanticException("if condition must be boolean");
    node.if_branch.second->accept(*this);
    if (node.else_branch) {
        node.else_branch->accept(*this);
    }

    VISIT_BODY_END
}

void Analyzer::visit(WhileStatement& node) {
    VISIT_BODY_BEGIN
    node.condition->accept(*this);
    if (!dynamic_cast<BoolType*>(current_type.get()))
        throw SemanticException("while condition must be boolean");

    node.statement->accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(ForStatement& node) {
    VISIT_BODY_BEGIN

    if (node.initialization)
        node.initialization->accept(*this);

    if (node.condition) {
        node.condition->accept(*this);
        if (!dynamic_cast<BoolType*>(current_type.get()))
            throw SemanticException("for condition must be boolean");
    }

    if (node.increment)
        node.increment->accept(*this);

    node.body->accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(ReturnStatement& node) {
    VISIT_BODY_BEGIN


    // Если мы в режиме дедукции, просто собираем типы и строго проверяем совпадение
    if (is_deducing_return) {
        if (node.expression) {
            node.expression->accept(*this);
            // Убираем const-обёртку, если есть
            std::shared_ptr<Type> expr_base = current_type;
            if (auto cp = dynamic_cast<ConstType*>(current_type.get())) {
                expr_base = cp->get_base();
            }

            if (!deduced_return_type) {
                // Первый return — запомним его тип
                deduced_return_type = expr_base;
            } else {
                // Последующие return должны строго совпадать с уже запомненным
                if (!deduced_return_type->equals(expr_base)) {
                    throw SemanticException(
                        "deduced return type  conflicts with type in auto-function"
                    );
                }
            }
        } else {
            // «return;» без выражения → void
            auto voidType = Analyzer::default_types.at("void");
            if (!deduced_return_type) {
                deduced_return_type = voidType;
            } else {
                // Если уже был какой-то не-void, то конфликт
                if (!dynamic_cast<VoidType*>(deduced_return_type.get())) {
                    throw SemanticException(
                        "deduced return type conflicts with void in auto-function"
                    );
                }
            }
        }
        return;
    }

    // Обычная валидация (не дедукция)
    if (return_type_stack.empty())
        throw SemanticException("return outside of function");

    // Ожидаемый тип — верхушка стека
    auto declared = return_type_stack.back();
    std::shared_ptr<Type> declared_base = declared;
    if (auto cp = dynamic_cast<ConstType*>(declared.get())) {
        declared_base = cp->get_base();
    }

    if (node.expression) {
        node.expression->accept(*this);
        auto expr_t = current_type;
        std::shared_ptr<Type> expr_base = expr_t;
        if (auto cp = dynamic_cast<ConstType*>(expr_t.get())) {
            expr_base = cp->get_base();
        }

        // Строгое равенство типов
        if (!expr_base->equals(declared_base)) {
            throw SemanticException(
                "return type mismatch: cannot convert "
            );
        }
        current_type = declared_base;
    } else {
        // «return;» без expr → только в void-функции
        if (!dynamic_cast<VoidType*>(declared_base.get())) {
            throw SemanticException("non-void function must return a value");
        }
        current_type = declared_base;
    }

    VISIT_BODY_END
}



void Analyzer::visit(BreakStatement& /*node*/) {}
void Analyzer::visit(ContinueStatement& /*node*/) {}
void Analyzer::visit(BinaryOperation& node) {
    VISIT_BODY_BEGIN

    
    if (node.op == "=") {
        node.lhs->accept(*this);
        auto lhs_t = current_type;
        if (dynamic_cast<ConstType*>(lhs_t.get())) {
            throw SemanticException("assignment to const variable");
        }
        node.rhs->accept(*this);
        auto rhs_t = current_type;
        if (!rhs_t->equals(lhs_t) &&
            !(dynamic_cast<Arithmetic*>(rhs_t.get()) &&
              dynamic_cast<Arithmetic*>(lhs_t.get()))) {
            throw SemanticException("type mismatch in assignment");
        }
        current_type = lhs_t;

    
    }
    if (node.op == "<"  || node.op == ">"  ||
        node.op == "<=" || node.op == ">=" ||
        node.op == "==" || node.op == "!=") {
        node.lhs->accept(*this);
        auto left = current_type;
        node.rhs->accept(*this);
        auto right = current_type;

        // оба операнда должны быть арифметическими
        if (!dynamic_cast<Arithmetic*>(left.get()) ||
            !dynamic_cast<Arithmetic*>(right.get())) {
            throw SemanticException("binary comparison requires arithmetic types");
        }
        // результат сравнения всегда bool
        current_type = Analyzer::default_types.at("bool");
        return;
    }

    

    node.lhs->accept(*this);
    auto left = current_type;
    node.rhs->accept(*this);
    auto right = current_type;

    if (!dynamic_cast<Arithmetic*>(left.get()) ||
        !dynamic_cast<Arithmetic*>(right.get())) {
        throw SemanticException("binary operation requires arithmetic types");
    }

    int rl = getTypeRank(*left);
    int rr = getTypeRank(*right);
    if (rl < 0 || rr < 0) {
        throw SemanticException("type mismatch in binary operation");
    }

    current_type = compareRank(left, right);

    VISIT_BODY_END
}


void Analyzer::visit(PrefixExpression& node) {
    VISIT_BODY_BEGIN
    node.base->accept(*this);
    if (dynamic_cast<Arithmetic*>(current_type.get()) == nullptr)
        throw SemanticException("invalid type for prefix operation");
    VISIT_BODY_END
}

void Analyzer::visit(PostfixIncrementExpression& node) {
    VISIT_BODY_BEGIN
    node.base->accept(*this);
    if (dynamic_cast<Arithmetic*>(current_type.get()) == nullptr)
        throw SemanticException("invalid type for postfix increment");
    VISIT_BODY_END
}

void Analyzer::visit(PostfixDecrementExpression& node) {
    VISIT_BODY_BEGIN
    node.base->accept(*this);
    if (dynamic_cast<Arithmetic*>(current_type.get()) == nullptr)
        throw SemanticException("invalid type for postfix decrement");
    VISIT_BODY_END
}

void Analyzer::visit(FunctionCallExpression& node) {
    VISIT_BODY_BEGIN

    // 1) Собираем фактические типы аргументов
    std::vector<std::shared_ptr<Type>> arg_types;
    for (auto& arg : node.args) {
        arg->accept(*this);
        auto t = current_type;
        if (auto cp = dynamic_cast<ConstType*>(t.get()))
            t = cp->get_base();
        arg_types.push_back(t);
    }

    std::shared_ptr<FuncType> func_t;

    // 2) Вызов метода структуры: obj.method(...)
    if (auto mexpr = dynamic_cast<StructMemberAccessExpression*>(node.base.get())) {
        mexpr->accept(*this);
        func_t = std::dynamic_pointer_cast<FuncType>(current_type);
        if (!func_t)
            throw SemanticException("expression is not a method");

        // (тут можно проверить совпадение сигнатуры, если нужно)

    // 3) Простой свободный вызов: f(...)
    } else if (auto ident = dynamic_cast<IdentifierExpression*>(node.base.get())) {
        // Перебираем все символы с таким именем
        for (auto& sym : scope->match_range(ident->name)) {
            auto fs = std::dynamic_pointer_cast<FuncSymbol>(sym);
            if (!fs) continue;

            // Явно приводим к FuncType
            auto ftype = std::dynamic_pointer_cast<FuncType>(fs->type);
            if (!ftype) continue;

            // Проверяем параметры
            const auto& params = ftype->get_args();
            if (params.size() != arg_types.size()) 
                continue;

            bool match = true;
            for (size_t i = 0; i < params.size(); ++i) {
                if (!arg_types[i]->equals(params[i])) {
                    match = false;
                    break;
                }
            }
            if (match) {
                func_t = ftype;
                break;
            }
        }
        if (!func_t)
            throw SemanticException("undefined function or signature mismatch: " + ident->name);

    // 4) Вызов через выражение: (expr)(...)
    } else {
        node.base->accept(*this);
        func_t = std::dynamic_pointer_cast<FuncType>(current_type);
        if (!func_t)
            throw SemanticException("expression is not a function");

        const auto& params = func_t->get_args();
        if (params.size() != arg_types.size())
            throw SemanticException("argument count mismatch");
        for (size_t i = 0; i < params.size(); ++i) {
            if (!arg_types[i]->equals(params[i]))
                throw SemanticException("argument type mismatch");
        }
    }

    // 5) Устанавливаем возвращаемый тип
    current_type = func_t->get_returnable_type();

    VISIT_BODY_END
}


void Analyzer::visit(SubscriptExpression& node) {
    VISIT_BODY_BEGIN
    node.base->accept(*this);
    auto arr_t = std::dynamic_pointer_cast<ArrayType>(current_type);
    if (!arr_t)
        throw SemanticException("expression is not an array");
    node.index->accept(*this);
    if (dynamic_cast<Integral*>(current_type.get()) == nullptr)
        throw SemanticException("index must be an integer");
    current_type = arr_t->get_base_type();
    VISIT_BODY_END
}

void Analyzer::visit(IdentifierExpression& node) {
    VISIT_BODY_BEGIN

    std::shared_ptr<Symbol> sym;
    try {
        
        sym = scope->match_global(node.name);
    } catch (const std::runtime_error&) {
        throw SemanticException("undefined variable: " + node.name);
    }


    auto varSym = std::dynamic_pointer_cast<VarSymbol>(sym);
    if (!varSym) {
        throw SemanticException(node.name + " is not a variable");
    }


    current_type = varSym->type;

    VISIT_BODY_END
}
void Analyzer::visit(IntLiteral& node){   
    VISIT_BODY_BEGIN
    current_type = std::make_shared<IntegerType>(static_cast<int8_t>(node.value)); 
    VISIT_BODY_END
}
void Analyzer::visit(FloatLiteral& node) { 
    VISIT_BODY_BEGIN
     current_type = std::make_shared<FloatType>(static_cast<double>(node.value));
    VISIT_BODY_END
}
void Analyzer::visit(CharLiteral& node){ 
    VISIT_BODY_BEGIN
    current_type = std::make_shared<CharType>(static_cast<char16_t>(node.value)); 
    VISIT_BODY_END
}
void Analyzer::visit(StringLiteral& node) { 
    VISIT_BODY_BEGIN
    current_type = std::make_shared<StringType>(node.value); 
    VISIT_BODY_END
}
void Analyzer::visit(BoolLiteral& node) { 
    VISIT_BODY_BEGIN
    current_type = std::make_shared<BoolType>(node.value); 
    VISIT_BODY_END
}

void Analyzer::visit(ParenthesizedExpression& node) {
    VISIT_BODY_BEGIN
    node.expression->accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(TernaryExpression& node) {
    VISIT_BODY_BEGIN
    node.condition->accept(*this);
    if (!dynamic_cast<BoolType*>(current_type.get()))
        throw SemanticException("ternary condition must be boolean");

    node.true_expr->accept(*this);
    auto left = current_type;
    node.false_expr->accept(*this);
    auto right = current_type;

    if (dynamic_cast<Arithmetic*>(left.get()) == nullptr
     || dynamic_cast<Arithmetic*>(right.get()) == nullptr) {
        throw SemanticException("ternary expression requires arithmetic types");
    }
    int rl = getTypeRank(*left);
    int rr = getTypeRank(*right);
    if (rl < 0 || rr < 0) {
        throw SemanticException("ternary expression types do not match");
    }

    current_type = compareRank(left, right);
    VISIT_BODY_END
}

void Analyzer::visit(StructMemberAccessExpression& node) {
    VISIT_BODY_BEGIN

    // сначала получаем тип базового выражения
    node.base->accept(*this);

    // распаковываем возможный const-обёртку
    bool obj_const = false;
    std::shared_ptr<Type> obj_t = current_type;
    if (auto cp = dynamic_cast<ConstType*>(obj_t.get())) {
        obj_const = true;
        obj_t = cp->get_base();
    }

    // проверяем, что это действительно StructType
    auto struct_t = std::dynamic_pointer_cast<StructType>(obj_t);
    if (!struct_t) {
        throw SemanticException("expression is not a struct");
    }

    // сначала пробуем поле
    const auto& members = struct_t->get_members();
    auto fit = members.find(node.member);
    if (fit != members.end()) {
        current_type = fit->second;
        return;
    }

    // затем пробуем метод
    const auto& methods = struct_t->get_methods();
    auto mit = methods.find(node.member);
    if (mit != methods.end()) {
        auto func_t = mit->second;
        // если объект const, метод тоже должен быть const
        if (obj_const && !func_t->is_method_const()) {
            throw SemanticException(
                "cannot call non-const method on const object");
        }
        current_type = func_t;
        return;
    }

    throw SemanticException("struct does not have member: " + node.member);

    VISIT_BODY_END
}


void Analyzer::visit(SizeOfExpression& node) {
    VISIT_BODY_BEGIN
    if (node.is_type) {
        current_type = get_type(node.type_name);
    } else {
        node.expression->accept(*this);
        current_type = std::make_shared<IntegerType>(sizeof(void*));
    }
    VISIT_BODY_END
}

std::shared_ptr<Type> Analyzer::get_type(const std::string& name) {
    std::shared_ptr<Symbol> sym;
    try {
        sym = scope->match_global(name);
    } catch (const std::runtime_error&) {
        // 2) Если не нашли — пробуем встроенные типы
        auto it = default_types.find(name);
        if (it != default_types.end())
            return it->second;
        throw SemanticException("unknown type: " + name);
    }

    // 3) Должен быть StructSymbol
    if (auto structSym = std::dynamic_pointer_cast<StructSymbol>(sym)) {
        return structSym->type;
    }

    // 4) Иначе — не тот символ
    throw SemanticException("symbol '" + name + "' is not a struct type");
}

void Analyzer::visit(DoWhileStatement& node) {
    VISIT_BODY_BEGIN

    node.statement->accept(*this);
    node.condition->accept(*this);
    if (!dynamic_cast<BoolType*>(current_type.get()))
        throw SemanticException("do-while condition must be boolean");

    VISIT_BODY_END
}


// ————————— NameSpaceAcceptExpression —————————
void Analyzer::visit(NameSpaceAcceptExpression& node) {
    VISIT_BODY_BEGIN

    // 1) Разрешаем левую часть — должно быть имя namespace
    auto baseId = dynamic_cast<IdentifierExpression*>(node.base.get());
    if (!baseId)
        throw SemanticException("left side of '::' must be a namespace name");

    std::shared_ptr<Symbol> sym;
    try {
        sym = scope->match_global(baseId->name);
    } catch (const std::runtime_error&) {
        throw SemanticException("undefined namespace: " + baseId->name);
    }

    // 2) Проверяем, что это NamespaceSymbol
    auto nsSym = std::dynamic_pointer_cast<NamespaceSymbol>(sym);
    if (!nsSym)
        throw SemanticException(baseId->name + " is not a namespace");

    // 3) Входим в область видимости этого namespace
    auto saved = scope;
    scope = nsSym->scope;

    // 4) Ищем требуемый символ (переменную или структуру) внутри namespace
    std::shared_ptr<Symbol> member;
    try {
        member = scope->match_global(node.name);
    } catch (const std::runtime_error&) {
        throw SemanticException("undefined symbol: " + node.name +
                                " in namespace " + baseId->name);
    }

    // 5) В зависимости от типа символа, выставляем current_type
    if (auto varSym = std::dynamic_pointer_cast<VarSymbol>(member)) {
        current_type = varSym->type;
    }
    else if (auto structSym = std::dynamic_pointer_cast<StructSymbol>(member)) {
        current_type = structSym->type;
    }
    else {
        throw SemanticException("symbol '" + node.name +
                                "' in namespace is not a variable or struct");
    }

    // 6) Выходим обратно
    scope = saved;

    VISIT_BODY_END
}


void Analyzer::visit(StaticAssertStatement& node) {
    VISIT_BODY_BEGIN
    node.condition->accept(*this);
    if (!dynamic_cast<BoolType*>(current_type.get()) &&
        !dynamic_cast<Integral*>(current_type.get()))
        throw SemanticException("static_assert requires constant boolean/integral");
    if (!evaluateConstant(node.condition.get()))
        throw SemanticException("static assertion failed: " + node.msg);
    VISIT_BODY_END
}

bool Analyzer::evaluateConstant(ASTNode* expr) {
    if (auto b = dynamic_cast<BoolLiteral*>(expr)) {
        return b->value;
    }
    if (auto i = dynamic_cast<IntLiteral*>(expr)) {
        return i->value != 0;
    }
    if (auto bin = dynamic_cast<BinaryOperation*>(expr)) {
        bool l = evaluateConstant(bin->lhs.get());
        bool r = evaluateConstant(bin->rhs.get());
        
        if (bin->op == "+")        return l + r;
        else if (bin->op == "-")   return l - r;
        else if (bin->op == "*")   return l * r;
        else if (bin->op == "/")   return r ? (l / r) : false;
        
        else if (bin->op == "<")   return l < r;
        else if (bin->op == ">")   return l > r;
        else if (bin->op == "<=")  return l <= r;
        else if (bin->op == ">=")  return l >= r;
        else if (bin->op == "==")  return l == r;
        else if (bin->op == "!=")  return l != r;

        else if (bin->op == "&&")  return l && r;
        else if (bin->op == "||")  return l || r;

        throw SemanticException("unsupported operator in static_assert: " + bin->op);
    }
    throw SemanticException("static_assert requires compile-time constant expression");
}




// 1 tip 
// strogaya
// 2 tip poka ssylki v const i td
// 3

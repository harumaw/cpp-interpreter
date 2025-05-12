
#include "analyzer.hpp"
#include "token.hpp"
#include <stdexcept>
#include "semantic_exception.hpp"

std::unordered_map<std::string, std::shared_ptr<Type>> Analyzer::default_types = {
    {"int",    std::make_shared<IntegerType>()},
    {"float",  std::make_shared<FloatType>()},
    {"double", std::make_shared<FloatType>()},
    {"char",   std::make_shared<CharType>()},
    {"bool",   std::make_shared<BoolType>()}
};

Analyzer::Analyzer()
  : scope(std::make_shared<Scope>(nullptr, nullptr))
  , current_type(nullptr)
{}

void Analyzer::analyze(TranslationUnit& unit) {
    scope = std::make_shared<Scope>(nullptr, nullptr);
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
    for (auto& node : unit.get_nodes()) {
        visit(*node);
    }
}

void Analyzer::visit(ASTNode& node) {
    node.accept(*this);
}

void Analyzer::visit(Declaration::SimpleDeclarator& node) {
}

void Analyzer::visit(Declaration::PtrDeclarator& node) {
    current_type = std::make_shared<PointerType>(current_type);
}

void Analyzer::visit(Declaration::InitDeclarator& node) {
    node.declarator->accept(*this);
    if (node.initializer)
        node.initializer->accept(*this);
}

void Analyzer::visit(VarDeclaration& node) {
    auto declared = get_type(node.type);
    for (auto& decl : node.declarator_list) {
        current_type = declared;
        decl->declarator->accept(*this);
        const auto& name = decl->declarator->name;
        if (scope->has_variable(name))
            throw SemanticException("variable already declared: " + name);
        scope->push_variable(name, current_type);
        if (decl->initializer)
            decl->initializer->accept(*this);
    }
}

void Analyzer::visit(ParameterDeclaration& node) {
    auto declared = get_type(node.type);
    current_type = declared;
    node.init_declarator->declarator->accept(*this);
    const auto& name = node.init_declarator->declarator->name;
    if (scope->has_variable(name))
        throw SemanticException("parameter already declared: " + name);
    scope->push_variable(name, current_type);
    if (node.init_declarator->initializer)
        node.init_declarator->initializer->accept(*this);
}
void Analyzer::visit(FuncDeclaration& node) {
    // 1. Получаем возвращаемый тип
    auto return_t = get_type(node.type);

    // 2. Собираем типы параметров (но ещё не входим в тело)
    std::vector<std::shared_ptr<Type>> arg_types;
    for (auto& param : node.args) {
        // Аналогично VarDeclaration: сбрасываем current_type на базовый
        current_type = get_type(param->type);
        // Разбираем сам declarator (Simple или Ptr)
        param->init_declarator->declarator->accept(*this);
        arg_types.push_back(current_type);
    }

    // 3. Создаём FuncType и регистрируем её в родительском скоупе
    auto func_t = std::make_shared<FuncType>(return_t, arg_types);
    scope->push_func(node.declarator->name, func_t);

    // 4. Заходим в новый скоуп для тела функции
    scope = scope->create_new_table(scope, node.body);

    // 5. Регистрируем параметры как переменные в теле
    for (size_t i = 0; i < node.args.size(); ++i) {
        const auto& pname = node.args[i]->init_declarator->declarator->name;
        if (scope->has_variable(pname))
            throw SemanticException("parameter already declared: " + pname);
        scope->push_variable(pname, arg_types[i]);
    }

    // 6. Разбираем тело
    node.body->accept(*this);

    // 7. Возвращаемся из скоупа
    scope = scope->get_prev_table();

    // 8. Отдаём в current_type сам FuncType
    current_type = func_t;
}

void Analyzer::visit(StructDeclaration& node) {
    std::unordered_map<std::string, std::shared_ptr<Type>> members;
    for (auto& m : node.members) {
        m->accept(*this);
        const auto& name = m->declarator_list[0]->declarator->name;
        members[name] = current_type;
    }
    auto st = std::make_shared<StructType>(members);
    scope->push_struct(node.name, st);
    current_type = st;
}

void Analyzer::visit(ArrayDeclaration& node) {
    node.size->accept(*this);
    if (!dynamic_cast<Integral*>(current_type.get()))
        throw SemanticException("array size must be integer");
    auto base_t = get_type(node.type);
    auto arr_t = std::make_shared<ArrayType>(base_t, node.size);
    if (scope->has_variable(node.name))
        throw SemanticException("variable already declared: " + node.name);
    scope->push_variable(node.name, arr_t);
    current_type = arr_t;
}


void Analyzer::visit(CompoundStatement& node) {
    scope = scope->create_new_table(scope, std::make_shared<CompoundStatement>(node));
    for (auto& s : node.statements)
        s->accept(*this);
    scope = scope->get_prev_table();
}

void Analyzer::visit(DeclarationStatement& node) {
    node.declaration->accept(*this);
}

void Analyzer::visit(ExpressionStatement& node) {
    node.expression->accept(*this);
}

void Analyzer::visit(ConditionalStatement& node) {
    node.if_branch.first->accept(*this);
    node.if_branch.second->accept(*this);
    if (node.else_branch)
        node.else_branch->accept(*this);
}

void Analyzer::visit(WhileStatement& node) {
    node.condition->accept(*this);
    node.statement->accept(*this);
}

void Analyzer::visit(ForStatement& node) {
    if (node.initialization)
        node.initialization->accept(*this);

    if (node.condition)
        node.condition->accept(*this);

    if (node.increment)
        node.increment->accept(*this);
        
    node.body->accept(*this);
}

void Analyzer::visit(ReturnStatement& node) {
    if (node.expression)
        node.expression->accept(*this);
}

void Analyzer::visit(BreakStatement& /*node*/) {}
void Analyzer::visit(ContinueStatement& /*node*/) {}


void Analyzer::visit(BinaryOperation& node) {
    node.lhs->accept(*this);
    auto left = current_type;
    node.rhs->accept(*this);
    auto right = current_type;
    if (!left->equals(right) || dynamic_cast<Arithmetic*>(left.get()) == nullptr)
        throw SemanticException("type mismatch in binary operation");
    current_type = left;
}

void Analyzer::visit(PrefixExpression& node) {
    node.base->accept(*this);
    if (dynamic_cast<Arithmetic*>(current_type.get()) == nullptr)
        throw SemanticException("invalid type for prefix operation");
}

void Analyzer::visit(PostfixIncrementExpression& node) {
    node.base->accept(*this);
    if (dynamic_cast<Arithmetic*>(current_type.get()) == nullptr)
        throw SemanticException("invalid type for postfix increment");
}

void Analyzer::visit(PostfixDecrementExpression& node) {
    node.base->accept(*this);
    if (dynamic_cast<Arithmetic*>(current_type.get()) == nullptr)
        throw SemanticException("invalid type for postfix decrement");
}

void Analyzer::visit(FunctionCallExpression& node) {
    // 1) Сначала проходим по всем аргументам и собираем их типы
    std::vector<std::shared_ptr<Type>> arg_types;
    for (auto& arg : node.args) {
        arg->accept(*this);
        arg_types.push_back(current_type);
    }

    // 2) Ищем функцию: если базовое выражение — просто идентификатор, 
    //    то используем match_function, иначе пытаемся взять из current_type
    std::shared_ptr<FuncType> func_t;
    if (auto ident = dynamic_cast<IdentifierExpression*>(node.base.get())) {
        // имя функции в виде идентификатора
        func_t = scope->match_function(ident->name, arg_types);
    } else {
        // вычисляем выражение, надеясь на функцию-указатель или лямбду
        node.base->accept(*this);
        func_t = std::dynamic_pointer_cast<FuncType>(current_type);
        if (!func_t) {
            throw SemanticException("expression is not a function");
        }
        // здесь мы тоже проверяем count и типы, на всякий случай
        auto params = func_t->get_args();
        if (params.size() != arg_types.size()) {
            throw SemanticException("argument count mismatch");
        }
        for (size_t i = 0; i < arg_types.size(); ++i) {
            if (!arg_types[i]->equals(params[i])) {
                throw SemanticException("argument type mismatch");
            }
        }
    }

    // 3) Всё ок, запоминаем возвращаемый тип
    current_type = func_t->get_returnable_type();
}

void Analyzer::visit(SubscriptExpression& node) {
    node.base->accept(*this);
    auto arr_t = std::dynamic_pointer_cast<ArrayType>(current_type);
    if (!arr_t)
        throw SemanticException("expression is not an array");
    node.index->accept(*this);
    if (dynamic_cast<Integral*>(current_type.get()) == nullptr)
        throw SemanticException("index must be an integer");
    current_type = arr_t->get_base_type();
}

void Analyzer::visit(IdentifierExpression& node) {
    try {
        current_type = scope->match_variable(node.name);
    } catch (...) {
        throw SemanticException("undefined variable: " + node.name);
    }
}

void Analyzer::visit(IntLiteral& node){   
    current_type = std::make_shared<IntegerType>(static_cast<int8_t>(node.value)); 
}
void Analyzer::visit(FloatLiteral& node) { 
     current_type = std::make_shared<FloatType>(static_cast<double>(node.value));
}
void Analyzer::visit(CharLiteral& node){ 
    current_type = std::make_shared<CharType>(static_cast<char16_t>(node.value)); 
}
void Analyzer::visit(StringLiteral& node) { 
    current_type = std::make_shared<StringType>(node.value); 
}
void Analyzer::visit(BoolLiteral& node) { 
    current_type = std::make_shared<BoolType>(node.value); 
}

void Analyzer::visit(ParenthesizedExpression& node) {
    node.expression->accept(*this);
}

void Analyzer::visit(TernaryExpression& node) {
    node.condition->accept(*this);
    node.true_expr->accept(*this);
    auto t1 = current_type;
    node.false_expr->accept(*this);
    if (!t1->equals(current_type))
        throw SemanticException("ternary expression types do not match");
    current_type = t1;
}

void Analyzer::visit(StructMemberAccessExpression& node) {
    // сначала вычисляем тип базового выражения
    node.base->accept(*this);
    // проверяем, что это действительно StructType
    auto struct_t = std::dynamic_pointer_cast<StructType>(current_type);
    if (!struct_t) {
        throw SemanticException("expression is not a struct");
    }
    // достаём мапу полей
    auto members = struct_t->get_members();
    // ищем нужное имя
    auto it = members.find(node.member);
    if (it == members.end()) {    // 1) Сначала проходим по всем аргументам и собираем их типы
        throw SemanticException("struct does not have member: " + node.member);
    }
    // результатом обращения становится тип этого поля
    current_type = it->second;
}


void Analyzer::visit(SizeOfExpression& node) {
    if (node.is_type) {
        current_type = get_type(node.type_name);
    } else {
        node.expression->accept(*this);
        current_type = std::make_shared<IntegerType>(sizeof(void*));
    }
}

std::shared_ptr<Type> Analyzer::get_type(const std::string& name) {
    try {
        return scope->match_struct(name);
    } catch (...) {
        auto it = default_types.find(name);
        if (it != default_types.end()) return it->second;
    }
    throw SemanticException("unknown type: " + name);
}



void Analyzer::visit(DoWhileStatement& node) {
    node.statement->accept(*this);
    node.condition->accept(*this);
}
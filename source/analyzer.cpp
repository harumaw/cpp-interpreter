#include "analyzer.hpp"
#include "token.hpp"
#include "semantic_exception.hpp"



std::unordered_map<std::string, std::shared_ptr<Type>> Analyzer::default_types = {
    {"int",    std::make_shared<IntegerType>()},
    {"float",  std::make_shared<FloatType>()},
    {"char",   std::make_shared<CharType>()},
    {"bool",   std::make_shared<BoolType>()},
    {"void", std::make_shared<VoidType>()}
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
    VISIT_BODY_END
}

void Analyzer::visit(ParameterDeclaration& node) {
    VISIT_BODY_BEGIN
    auto declared = get_type(node.type);
    current_type = declared;
    node.init_declarator->declarator->accept(*this);
    const auto& name = node.init_declarator->declarator->name;
    if (scope->has_variable(name))
        throw SemanticException("parameter already declared: " + name);
    scope->push_variable(name, current_type);
    if (node.init_declarator->initializer)
        node.init_declarator->initializer->accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(FuncDeclaration& node) {
    VISIT_BODY_BEGIN
    auto return_t = get_type(node.type);


    std::vector<std::shared_ptr<Type>> arg_types;
    for (auto& param : node.args) {
        current_type = get_type(param->type);
        param->init_declarator->declarator->accept(*this);
        arg_types.push_back(current_type);
    }

    auto func_t = std::make_shared<FuncType>(return_t, arg_types);
    scope->push_func(node.declarator->name, func_t);
    
    scope = scope->create_new_table(scope, node.body);

    for (size_t i = 0; i < node.args.size(); ++i) {
        const auto& pname = node.args[i]->init_declarator->declarator->name;
        if (scope->has_variable(pname))
            throw SemanticException("parameter already declared: " + pname);
        scope->push_variable(pname, arg_types[i]);
    }


    node.body->accept(*this);
    scope = scope->get_prev_table();
    current_type = func_t;
    VISIT_BODY_END
}

void Analyzer::visit(StructDeclaration& node) {
    VISIT_BODY_BEGIN
    std::unordered_map<std::string, std::shared_ptr<Type>> members;
    for (auto& m : node.members) {
        m->accept(*this);
        const auto& name = m->declarator_list[0]->declarator->name;
        members[name] = current_type;
    }
    auto st = std::make_shared<StructType>(members);
    scope->push_struct(node.name, st);
    current_type = st;
    VISIT_BODY_END
}

void Analyzer::visit(ArrayDeclaration& node) {
    VISIT_BODY_BEGIN
    node.size->accept(*this);
    if (!dynamic_cast<Integral*>(current_type.get()))
        throw SemanticException("array size must be integer");
    auto base_t = get_type(node.type);
    auto arr_t = std::make_shared<ArrayType>(base_t, node.size);
    if (scope->has_variable(node.name))
        throw SemanticException("variable already declared: " + node.name);
    scope->push_variable(node.name, arr_t);
    current_type = arr_t;
    VISIT_BODY_END
}

void Analyzer::visit(CompoundStatement& node) {
    VISIT_BODY_BEGIN
    scope = scope->create_new_table(scope, std::make_shared<CompoundStatement>(node));
    for (auto& s : node.statements)
        s->accept(*this);
    scope = scope->get_prev_table();
    VISIT_BODY_END
}

void Analyzer::visit(NameSpaceDeclaration& node) {
    VISIT_BODY_BEGIN
    scope = scope->create_new_table(scope, std::make_shared<NameSpaceDeclaration>(node));
    for (auto& decl : node.declarations)
        decl->accept(*this);
    scope = scope->get_prev_table();
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
    node.if_branch.second->accept(*this);
    if (node.else_branch)
        node.else_branch->accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(WhileStatement& node) {
    VISIT_BODY_BEGIN
    node.condition->accept(*this);
    node.statement->accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(ForStatement& node) {
    VISIT_BODY_BEGIN
    if (node.initialization)
        node.initialization->accept(*this);
    if (node.condition)
        node.condition->accept(*this);
    if (node.increment)
        node.increment->accept(*this);
    node.body->accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(ReturnStatement& node) {
    VISIT_BODY_BEGIN
    if (node.expression)
        node.expression->accept(*this);
    VISIT_BODY_END
}

void Analyzer::visit(BreakStatement& /*node*/) {}
void Analyzer::visit(ContinueStatement& /*node*/) {}

void Analyzer::visit(BinaryOperation& node) {
    VISIT_BODY_BEGIN
    node.lhs->accept(*this);
    auto left = current_type;
    node.rhs->accept(*this);
    auto right = current_type;
    if (!left->equals(right) || dynamic_cast<Arithmetic*>(left.get()) == nullptr)
        throw SemanticException("type mismatch in binary operation");
    current_type = left;
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
    //  сначала проходим по всем аргументам и собираем их типы
    std::vector<std::shared_ptr<Type>> arg_types;
    for (auto& arg : node.args) {
        arg->accept(*this);
        arg_types.push_back(current_type);
    }

    // ищем функцию: если базовое выражение — просто идентификатор, 
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
    try {
        current_type = scope->match_variable(node.name);
    } catch (...) {
        throw SemanticException("undefined variable: " + node.name);
    }
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
    node.true_expr->accept(*this);
    auto t1 = current_type;
    node.false_expr->accept(*this);
    if (!t1->equals(current_type))
        throw SemanticException("ternary expression types do not match");
    current_type = t1;
    VISIT_BODY_END
}

void Analyzer::visit(StructMemberAccessExpression& node) {
    VISIT_BODY_BEGIN
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
    if (it == members.end()) {
        throw SemanticException("struct does not have member: " + node.member);
    }
    // результатом обращения становится тип этого поля
    current_type = it->second;
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
    try {
        return scope->match_struct(name);
    } catch (...) {
        auto it = default_types.find(name);
        if (it != default_types.end()) return it->second;
    }
    throw SemanticException("unknown type: " + name);
}

void Analyzer::visit(DoWhileStatement& node) {
    VISIT_BODY_BEGIN
    node.statement->accept(*this);
    node.condition->accept(*this);
    VISIT_BODY_END
}

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

    if(node.is_const){
        declared = std::make_shared<ConstType>(declared);
    }
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

    // определяем возвращаемый тип функции
    auto return_t = get_type(node.type);

    // собираем типы параметров
    std::vector<std::shared_ptr<Type>> arg_types;
    for (auto& param : node.args) {
        current_type = get_type(param->type);
        param->init_declarator->declarator->accept(*this);
        arg_types.push_back(current_type);
    }

    // проверяем, не объявлена ли функция с такой сигнатурой в этом скоупе
    if (scope->has_function(node.declarator->name, arg_types)) {
        throw SemanticException("function already declared: " + node.declarator->name);
    }

    // регистрируем новую функцию в текущем скоупе
    auto func_t = std::make_shared<FuncType>(return_t, arg_types);
    scope->push_func(node.declarator->name, func_t);

    // сохраняем ожидаемый возвращаемый тип в стек
    return_type_stack.push_back(return_t);

    // переходим в новый вложенный скоуп для тела функции
    scope = scope->create_new_table(scope, node.body);

    // регистрируем параметры как локальные переменные
    for (size_t i = 0; i < node.args.size(); ++i) {
        const auto& pname = node.args[i]->init_declarator->declarator->name;
        if (scope->has_variable(pname)) {
            throw SemanticException("parameter already declared: " + pname);
        }
        scope->push_variable(pname, arg_types[i]);
    }


    node.body->accept(*this);

    // выходим из скоупа и восстанавливаем предыдущий ожидаемый тип
    scope = scope->get_prev_table();
    return_type_stack.pop_back();

    // устанавливаем current_type на тип функции (для вложенных вызовов)
    current_type = func_t;

    VISIT_BODY_END
}

void Analyzer::visit(StructDeclaration& node) {
    VISIT_BODY_BEGIN

    if (scope->has_variable(node.name)) {
        throw SemanticException("struct already declared: " + node.name);
    }
    std::unordered_map<std::string, std::shared_ptr<Type>> members;
    if (node.members.empty()) {
        throw SemanticException("struct must have at least one member: " + node.name);
    }
    for (auto& m : node.members) {
        m->accept(*this);
        const auto& name = m->declarator_list[0]->declarator->name;
        if (members.count(name)) {
            throw SemanticException("duplicate struct member: " + name + " in struct " + node.name);
        }

        if (!current_type) {
            throw SemanticException("unknown type for struct member: " + name + " in struct " + node.name);
        }

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

    if (scope->has_namespace(node.name))
        throw SemanticException("namespace already declared: " + node.name);

    auto ns_scope = scope->create_new_table(scope, nullptr);
    scope->push_namespace(node.name, ns_scope);

    auto saved = scope;
    scope = ns_scope;
    for (auto& decl : node.declarations)
        decl->accept(*this);
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

void Analyzer::visit(ReturnStatement& node) { //maybe rework
    VISIT_BODY_BEGIN
    if (return_type_stack.empty())
        throw SemanticException("return outside of function");

    if (node.expression) {
        node.expression->accept(*this);
        if (!current_type->equals(return_type_stack.back()))
            throw SemanticException("return type mismatch");
    } else {
        if (!dynamic_cast<VoidType*>(return_type_stack.back().get()))
            throw SemanticException("non-void function must return a value");
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
    std::vector<std::shared_ptr<Type>> arg_types;
    for (auto& arg : node.args) {
        arg->accept(*this);
        arg_types.push_back(current_type);
    }

    std::shared_ptr<FuncType> func_t;
    if (auto ident = dynamic_cast<IdentifierExpression*>(node.base.get())) {
        if (!scope->has_function(ident->name, arg_types))
            throw SemanticException("undefined function or signature mismatch: " + ident->name);
        func_t = scope->match_function(ident->name, arg_types);
    } else {
        node.base->accept(*this);
        func_t = std::dynamic_pointer_cast<FuncType>(current_type);
        if (!func_t)
            throw SemanticException("expression is not a function");
        auto params = func_t->get_args();
        if (params.size() != arg_types.size())
            throw SemanticException("argument count mismatch");
        for (size_t i = 0; i < arg_types.size(); ++i) {
            if (!arg_types[i]->equals(params[i]))
                throw SemanticException("argument type mismatch");
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
    if (scope->has_struct(name)) {
        return scope->match_struct(name);
    }
    auto it = default_types.find(name);
    if (it != default_types.end()) {
        return it->second;
    }
    throw SemanticException("unknown type: " + name);
}

void Analyzer::visit(DoWhileStatement& node) {
    VISIT_BODY_BEGIN

    node.statement->accept(*this);
    node.condition->accept(*this);
    if (!dynamic_cast<BoolType*>(current_type.get()))
        throw SemanticException("do-while condition must be boolean");

    VISIT_BODY_END
}


void Analyzer::visit(NameSpaceAcceptExpression& node) {
    VISIT_BODY_BEGIN
    auto ns_ident = dynamic_cast<IdentifierExpression*>(node.base.get());
    if (!ns_ident)
        throw SemanticException("left side of '::' must be a namespace name");

    if (!scope->has_namespace(ns_ident->name))
        throw SemanticException("undefined namespace: " + ns_ident->name);

    auto saved_scope = scope;
    scope = scope->match_namespace(ns_ident->name);
    current_type = scope->match_variable(node.name);
    scope = saved_scope;
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
//
#pragma once

#include "visitor.hpp"
#include "symbol.hpp"
#include "scope.hpp"

#include <any>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

class Execute : public Visitor {
public:
    Execute();
    ~Execute();

    void execute(TranslationUnit& unit);
    void visit(ASTNode&) override;
    void visit(TranslationUnit& unit) override;
    void visit(Declaration::PtrDeclarator&) override;
    void visit(Declaration::SimpleDeclarator&) override;
    void visit(Declaration::InitDeclarator&) override;
    void visit(VarDeclaration&) override;
    void visit(ParameterDeclaration&) override;
    void visit(FuncDeclaration&) override;
    void visit(StructDeclaration&) override;
    void visit(ArrayDeclaration&) override;
    void visit(NameSpaceDeclaration&) override;

    void visit(CompoundStatement&) override;
    void visit(DeclarationStatement&) override;
    void visit(ExpressionStatement&) override;
    void visit(ConditionalStatement&) override;
    void visit(WhileStatement&) override;
    void visit(ForStatement&) override;
    void visit(ReturnStatement&) override;
    void visit(BreakStatement&) override;
    void visit(ContinueStatement&) override;
    void visit(StructMemberAccessExpression&) override;
    void visit(DoWhileStatement&) override;

    void visit(BinaryOperation&) override;
    void visit(PrefixExpression&) override;
    void visit(PostfixIncrementExpression&) override;
    void visit(PostfixDecrementExpression&) override;
    void visit(FunctionCallExpression&) override;
    void visit(SubscriptExpression&) override;
    void visit(IntLiteral&) override;
    void visit(FloatLiteral&) override;
    void visit(CharLiteral&) override;
    void visit(StringLiteral&) override;
    void visit(BoolLiteral&) override;
    void visit(IdentifierExpression&) override;
    void visit(ParenthesizedExpression&) override;
    void visit(TernaryExpression&) override;
    void visit(SizeOfExpression&) override;
    void visit(NameSpaceAcceptExpression&) override;
    void visit(StaticAssertStatement&) override;

    std::shared_ptr<Scope> symbolTable;

private:

    std::shared_ptr<Symbol> match_symbol (const std::string& token);
    bool is_record_type(const std::shared_ptr<Type>& type);
    bool count_bool(std::any, std::string&, std::any);
    std::shared_ptr<VarSymbol> binary_operation(std::shared_ptr<VarSymbol>, std::string&, std::shared_ptr<VarSymbol>);
    std::shared_ptr<VarSymbol> unary_operation(std::shared_ptr<VarSymbol>, std::string&);
    std::shared_ptr<VarSymbol> postfix_operation(std::shared_ptr<VarSymbol>, std::string&);
    bool can_convert(const std::shared_ptr<Type>& from, const std::shared_ptr<Type>& to);

    std::shared_ptr<Symbol> current_value;
    std::vector<std::shared_ptr<FuncType>> matched_functions;
    static std::unordered_map<std::string, std::shared_ptr<Symbol>> default_types;
};

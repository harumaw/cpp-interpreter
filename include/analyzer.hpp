#pragma once 

#include "ast.hpp"
#include "visitor.hpp"
#include "scope.hpp"
#include "type.hpp"



class Analyzer : public Visitor{
public:
	Analyzer();
	void analyze(TranslationUnit&);
public:
	void visit(ASTNode&) override;
	void visit(TranslationUnit& unit) override;
public:
	void visit(Declaration::PtrDeclarator&) override;
	void visit(Declaration::SimpleDeclarator&) override;
	void visit(Declaration::InitDeclarator&) override;
	void visit(VarDeclaration&) override;
	void visit(ParameterDeclaration&) override;
	void visit(FuncDeclaration&) override;
	void visit(StructDeclaration&) override;
	void visit(ArrayDeclaration&) override;
public:
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
public:
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


	std::shared_ptr<Type> get_type(const std::string&);
	static std::unordered_map<std::string, std::shared_ptr<Type>> default_types;
	std::shared_ptr<Scope> scope;
    std::shared_ptr<Type> current_type;
};

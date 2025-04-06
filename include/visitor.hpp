#pragma once

#include "ast.hpp"
#include "declaration.hpp"
#include "statement.hpp"
#include "expression.hpp"

class Visitor {
public:
	virtual ~Visitor() = default;
public:
	virtual void visit(TranslationUnit&) = 0;
public:
	virtual void visit(Declaration::PtrDeclarator&) = 0;
	virtual void visit(Declaration::NoPtrDeclarator&) = 0;
	virtual void visit(Declaration::InitDeclarator&) = 0;
	virtual void visit(VarDeclaration&) = 0;
	virtual void visit(ParameterDeclaration&) = 0;
	virtual void visit(FuncDeclaration&) = 0;
public:
	virtual void visit(CompoundStatement&) = 0;
	virtual void visit(DeclarationStatement&) = 0;
	virtual void visit(ExpressionStatement&) = 0;
	virtual void visit(ConditionalStatement&) = 0;
	virtual void visit(WhileStatement&) = 0;
	virtual void visit(RepeatStatement&) = 0;
	virtual void visit(ForStatement&) = 0;
	virtual void visit(ReturnStatement&) = 0;
	virtual void visit(BreakStatement&) = 0;
	virtual void visit(ContinueStatement&) = 0;
public:
	virtual void visit(BinaryOperation&) = 0;
	virtual void visit(PrefixExpression&) = 0;
	virtual void visit(PostfixIncrementExpression&) = 0;
	virtual void visit(PostfixDecrementExpression&) = 0;
	virtual void visit(FunctionCallExpression&) = 0;
	virtual void visit(SubscriptExpression&) = 0;
	virtual void visit(IntLiteral&) = 0;
	virtual void visit(FloatLiteral&) = 0;
	virtual void visit(CharLiteral&) = 0;
	virtual void visit(StringLiteral&) = 0;
	virtual void visit(BoolLiteral&) = 0;
	virtual void visit(IdentifierExpression&) = 0;
	virtual void visit(ParenthesizedExpression&) = 0;
};
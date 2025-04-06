#include "visitor.hpp"

class Printer : public Visitor {
public:
	void visit(TranslationUnit&) override;
public:
	void visit(Declaration::PtrDeclarator&) override;
	void visit(Declaration::NoPtrDeclarator&) override;
	void visit(Declaration::InitDeclarator&) override;
	void visit(VarDeclaration&) override;
	void visit(ParameterDeclaration&) override;
	void visit(FuncDeclaration&) override;
public:
	void visit(CompoundStatement&) override;
	void visit(DeclarationStatement&) override;
	void visit(ExpressionStatement&) override;
	void visit(ConditionalStatement&) override;
	void visit(WhileStatement&) override;
	void visit(RepeatStatement&) override;
	void visit(ForStatement&) override;
	void visit(ReturnStatement&) override;
	void visit(BreakStatement&) override;
	void visit(ContinueStatement&) override;
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
};
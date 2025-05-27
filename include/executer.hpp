#include "visitor.hpp"
#include "symboltable.hpp"

class Execute : public Visitor {
public:
    Execute();
	
	
    void execute(TranslationUnit& unit);
public:
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
	void visit(NameSpaceAcceptExpression&) override;

private:
	std::shared_ptr<SymbolTable> table;
	Value current_value;

};
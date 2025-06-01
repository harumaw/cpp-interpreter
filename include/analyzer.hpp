#pragma once 

#include "ast.hpp"
#include "visitor.hpp"
#include "scope.hpp"
#include "type.hpp"
#include <iostream>



#define VISIT_BODY_BEGIN try {
	#define VISIT_BODY_END                         \
		} catch (const SemanticException& e) {     \
		  errors.push_back(e.what());              \
		}




class Analyzer : public Visitor{
public:
	Analyzer();
	void analyze(TranslationUnit&);

	std::vector<std::string> errors;
	
	const std::vector<std::string>& getErrors() const { return errors; }
   
	void printErrors() const {
        for (auto& e : errors) {
            std::cerr << e << "\n";
        }
    }

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
	void visit(StaticAssertStatement&) override;
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

	std::shared_ptr<Type> get_type(const std::string&);
	static std::unordered_map<std::string, std::shared_ptr<Type>> default_types;
	std::shared_ptr<Scope> scope;
    std::shared_ptr<Type> current_type;
	std::vector<std::shared_ptr<Type>> return_type_stack;

	bool is_deducing_return = false;
    std::shared_ptr<Type> deduced_return_type = nullptr;

	
	bool evaluateConstant(ASTNode*);

	enum class BinaryOp{
		Add, Subtract, Multiply, Divide, Less, Greater, Equal
	};

};

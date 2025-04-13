#pragma once

#include <string>
#include <vector>
#include <memory>

#include "ast.hpp"



struct BinaryExpression: public Expression {
	virtual ~BinaryExpression() = default;
	virtual void accept(Visitor&) override = 0;

};

struct BinaryOperation: public BinaryExpression {
	std::string op;
	std::shared_ptr<Expression> lhs, rhs;

	BinaryOperation(std::string, std::shared_ptr<Expression>, std::shared_ptr<Expression>);
	void accept(Visitor&) override;
};

struct UnaryExpression: public BinaryExpression {
	virtual ~UnaryExpression() = default;
	virtual void accept(Visitor&) override = 0;
};

struct PrefixExpression: public UnaryExpression {
	std::string op;
	std::shared_ptr<Expression> base;

	PrefixExpression(std::string, std::shared_ptr<Expression>);
	void accept(Visitor&) override;
};


struct PostfixExpression: public UnaryExpression {
	virtual ~PostfixExpression() = default;
	virtual void accept(Visitor&) override = 0;
};

struct FunctionCallExpression: public PostfixExpression {
	std::shared_ptr<Expression> base;
	std::vector<std::shared_ptr<Expression>> args;

	FunctionCallExpression(std::shared_ptr<Expression>, const std::vector<std::shared_ptr<Expression>>&);
	void accept(Visitor&) override;
};


struct PostfixIncrementExpression: public PostfixExpression {
	std::shared_ptr<Expression> base;

	PostfixIncrementExpression(std::shared_ptr<Expression>);
	void accept(Visitor&) override;
};

struct PostfixDecrementExpression: public PostfixExpression {
	std::shared_ptr<Expression> base;

	PostfixDecrementExpression(std::shared_ptr<Expression>);
	void accept(Visitor&) override;
};

struct PrimaryExpression: public PostfixExpression {
	virtual ~PrimaryExpression() = default;
	virtual void accept(Visitor&) override = 0;
};

struct IdentifierExpression: public PrimaryExpression {
	std::string name;

	IdentifierExpression(const std::string&);
	void accept(Visitor&) override;
};

struct LiteralExpression: public PrimaryExpression {
	virtual ~LiteralExpression() = default;
	virtual void accept(Visitor&) override = 0;
};


struct IntLiteral: public LiteralExpression {
	int value;

	IntLiteral(const std::string&);
	void accept(Visitor&) override;
};

struct FloatLiteral: public LiteralExpression {
	float value;

	FloatLiteral(const std::string&);
	void accept(Visitor&) override;
};

struct CharLiteral: public LiteralExpression {
	char value;

	CharLiteral(const std::string&);
	void accept(Visitor&) override;
};

struct StringLiteral: public LiteralExpression {
	std::string value;

	StringLiteral(const std::string&);
	void accept(Visitor&) override;
};

struct BoolLiteral: public LiteralExpression {
	bool value;

	BoolLiteral(const std::string&);
	void accept(Visitor&) override;
};

struct ParenthesizedExpression: public PrimaryExpression {
	std::shared_ptr<Expression> expression;

	ParenthesizedExpression(std::shared_ptr<Expression>);
	void accept(Visitor&) override;
};

struct StructMemberAccessExpression : public PostfixExpression {
	std::shared_ptr<Expression> base;
	std::string member;

	StructMemberAccessExpression(std::shared_ptr<Expression>, const std::string&);
	
	void accept(Visitor&) override;
};

struct SubscriptExpression: public PostfixExpression {
	std::shared_ptr<Expression> base;
	std::shared_ptr<Expression> index;

	SubscriptExpression(std::shared_ptr<Expression>, std::shared_ptr<Expression>);
	void accept(Visitor&) override;
};



struct TernaryExpression : public Expression{
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Expression> true_expr;
	std::shared_ptr<Expression> false_expr;


	TernaryExpression(std::shared_ptr<Expression>, std::shared_ptr<Expression>, std::shared_ptr<Expression>);

	void accept(Visitor&) override;
};


using expr_ptr = std::shared_ptr<Expression>;
using binary_expression = std::shared_ptr<BinaryExpression>;
using unary_expression = std::shared_ptr<UnaryExpression>;
using postfix_expression = std::shared_ptr<PostfixExpression>;
using primary_expression = std::shared_ptr<PrimaryExpression>;
using parentsized_expression = std::shared_ptr<ParenthesizedExpression>;
using func_param = std::vector<std::shared_ptr<Expression>>;
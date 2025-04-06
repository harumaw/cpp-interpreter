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
	std::shared_ptr<BinaryExpression> lhs, rhs;

	BinaryOperation(const std::string&, const std::shared_ptr<BinaryExpression>&, const std::shared_ptr<BinaryExpression>&);
	void accept(Visitor&) override;
};

struct UnaryExpression: public BinaryExpression {
	virtual ~UnaryExpression() = default;
	virtual void accept(Visitor&) override = 0;
};

struct PrefixExpression: public UnaryExpression {
	std::string op;
	std::shared_ptr<UnaryExpression> base;

	PrefixExpression(const std::string&, const std::shared_ptr<UnaryExpression>&);
	void accept(Visitor&) override;
};


struct PostfixExpression: public UnaryExpression {
	virtual ~PostfixExpression() = default;
	virtual void accept(Visitor&) override = 0;
};

struct FunctionCallExpression: public PostfixExpression {
	std::shared_ptr<PostfixExpression> base;
	std::vector<std::shared_ptr<Expression>> args;

	FunctionCallExpression(const std::shared_ptr<PostfixExpression>&, const std::vector<std::shared_ptr<Expression>>&);
	void accept(Visitor&) override;
};

struct SubscriptExpression: public PostfixExpression {
	std::shared_ptr<PostfixExpression> base;
	std::shared_ptr<Expression> index;

	SubscriptExpression(const std::shared_ptr<PostfixExpression>&, const std::shared_ptr<Expression>&);
	void accept(Visitor&) override;
};

struct PostfixIncrementExpression: public PostfixExpression {
	std::shared_ptr<PostfixExpression> base;

	PostfixIncrementExpression(const std::shared_ptr<PostfixExpression>&);
	void accept(Visitor&) override;
};

struct PostfixDecrementExpression: public PostfixExpression {
	std::shared_ptr<PostfixExpression> base;

	PostfixDecrementExpression(const std::shared_ptr<PostfixExpression>&);
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

	ParenthesizedExpression(const std::shared_ptr<Expression>&);
	void accept(Visitor&) override;
};

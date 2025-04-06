#include "visitor.hpp"

BinaryOperation::BinaryOperation(
	const std::string& op, 
	const std::shared_ptr<BinaryExpression>& lhs,
	const std::shared_ptr<BinaryExpression>& rhs
	) : op(op), lhs(lhs), rhs(rhs) {}

void BinaryOperation::accept(Visitor& visitor) {
	visitor.visit(*this);
}

PrefixExpression::PrefixExpression(
	const std::string& op,
	const std::shared_ptr<UnaryExpression>& base
	) : op(op), base(base) {}

void PrefixExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}

FunctionCallExpression::FunctionCallExpression(
	const std::shared_ptr<PostfixExpression>& base,
	const std::vector<std::shared_ptr<Expression>>& args
	) : base(base), args(args) {}

void FunctionCallExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}

SubscriptExpression::SubscriptExpression(
	const std::shared_ptr<PostfixExpression>& base,
	const std::shared_ptr<Expression>& index
	) : base(base), index(index) {}

void SubscriptExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}

PostfixIncrementExpression::PostfixIncrementExpression(
	const std::shared_ptr<PostfixExpression>& base
	) : base(base) {}

void PostfixIncrementExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}

PostfixDecrementExpression::PostfixDecrementExpression(
	const std::shared_ptr<PostfixExpression>& base
	) : base(base) {}

void PostfixDecrementExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}

IdentifierExpression::IdentifierExpression(
	const std::string& name
	) : name(name) {}

void IdentifierExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}

IntLiteral::IntLiteral(
	const std::string& value
	) : value(std::stoi(value)) {}

void IntLiteral::accept(Visitor& visitor) {
	visitor.visit(*this);
}

FloatLiteral::FloatLiteral(
	const std::string& value
	) : value(std::stof(value)) {}

void FloatLiteral::accept(Visitor& visitor) {
	visitor.visit(*this);
}

CharLiteral::CharLiteral(
	const std::string& value
	) : value(value[0]) {}

void CharLiteral::accept(Visitor& visitor) {
	visitor.visit(*this);
}

StringLiteral::StringLiteral(
	const std::string& value
	) : value(value) {}

void StringLiteral::accept(Visitor& visitor) {
	visitor.visit(*this);
}

BoolLiteral::BoolLiteral(
	const std::string& value
	) : value(value == "true" ? true : false) {}

void BoolLiteral::accept(Visitor& visitor) {
	visitor.visit(*this);
}

ParenthesizedExpression::ParenthesizedExpression(
	const std::shared_ptr<Expression>& expression
	) : expression(expression) {}

void ParenthesizedExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}
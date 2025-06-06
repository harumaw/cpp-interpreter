#include "visitor.hpp"

BinaryOperation::BinaryOperation(
	std::string op, 
	std::shared_ptr<Expression> lhs,
	std::shared_ptr<Expression> rhs
	) : op(op), lhs(lhs), rhs(rhs) {}

void BinaryOperation::accept(Visitor& visitor) {
	visitor.visit(*this);
}

PrefixExpression::PrefixExpression(
	std::string op,
	std::shared_ptr<Expression> base
	) : op(op), base(base) {}

void PrefixExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}

FunctionCallExpression::FunctionCallExpression(
	std::shared_ptr<Expression> base,
	const std::vector<std::shared_ptr<Expression>>& args
	) : base(base), args(args) {}

void FunctionCallExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}

PostfixIncrementExpression::PostfixIncrementExpression(
	std::shared_ptr<Expression> base
	) : base(base) {}

void PostfixIncrementExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}

PostfixDecrementExpression::PostfixDecrementExpression(
	std::shared_ptr<Expression> base
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

void NullPtrLiteral::accept(Visitor& visitor) {
	visitor.visit(*this);
}

ParenthesizedExpression::ParenthesizedExpression(
	std::shared_ptr<Expression> expression
	) : expression(expression) {}

void ParenthesizedExpression::accept(Visitor& visitor) {
	visitor.visit(*this);
}


StructMemberAccessExpression::StructMemberAccessExpression
(
	std::shared_ptr<Expression> base, const std::string& member)
        : base(base), member(member) {}

void StructMemberAccessExpression::accept(Visitor& visitor){
        visitor.visit(*this);
    }
	
SubscriptExpression::SubscriptExpression(
	std::shared_ptr<Expression> base,
	std::shared_ptr<Expression> index
) : base(base), index(index) {}

void SubscriptExpression::accept(Visitor& visitor) {
		visitor.visit(*this);
	}
	

TernaryExpression::TernaryExpression(
	std::shared_ptr<Expression> condition,
	std::shared_ptr<Expression> true_expr,
	std::shared_ptr<Expression> false_expr
) : condition(condition) , true_expr(true_expr), false_expr(false_expr) {}

void TernaryExpression::accept(Visitor& visitor){
	visitor.visit(*this);
}

SizeOfExpression::SizeOfExpression(
	const std::string& type_name
) : is_type(true), type_name(type_name), expression(nullptr) {}

SizeOfExpression::SizeOfExpression(
	std::shared_ptr<Expression> expression
) : is_type(false), type_name(""), expression(expression) {}

void SizeOfExpression::accept(Visitor& visitor){
	visitor.visit(*this);
}


NameSpaceAcceptExpression::NameSpaceAcceptExpression(
	std::shared_ptr<Expression> base,
	const std::string& name
) : base(base), name(name) {}
void NameSpaceAcceptExpression::accept(Visitor& visitor){
	visitor.visit(*this);
}
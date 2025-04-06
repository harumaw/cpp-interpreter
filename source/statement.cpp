#include "visitor.hpp"

CompoundStatement::CompoundStatement(
	const std::vector<std::shared_ptr<Statement>>& statements
	) : statements(statements) {}

void CompoundStatement::accept(Visitor& visitor) {
	visitor.visit(*this);
}

ConditionalStatement::ConditionalStatement(
	const std::pair<std::shared_ptr<Expression>, std::shared_ptr<Statement>>& if_branch,
	const std::shared_ptr<Statement>& else_branch
	) : if_branch(if_branch), else_branch(else_branch) {}

void ConditionalStatement::accept(Visitor& visitor) {
	visitor.visit(*this);
}

WhileStatement::WhileStatement(
	const std::shared_ptr<Expression>& condition,
	const std::shared_ptr<Statement>& statement
	) : condition(condition), statement(statement) {}

void WhileStatement::accept(Visitor& visitor) {
	visitor.visit(*this);
}

RepeatStatement::RepeatStatement(
		const std::shared_ptr<Statement>& statement
	) : statement(statement) {}

void RepeatStatement::accept(Visitor& visitor) {
	visitor.visit(*this);
}

void ForStatement::accept(Visitor& visitor) {
	visitor.visit(*this);
}

ReturnStatement::ReturnStatement(
	const std::shared_ptr<Expression>& expression
	) : expression(expression) {}

void ReturnStatement::accept(Visitor& visitor) {
	visitor.visit(*this);
}

void BreakStatement::accept(Visitor& visitor) {
	visitor.visit(*this);
}

void ContinueStatement::accept(Visitor& visitor) {
	visitor.visit(*this);
}

DeclarationStatement::DeclarationStatement(
	const std::shared_ptr<VarDeclaration>& declaration
	) : declaration(declaration) {}

void DeclarationStatement::accept(Visitor& visitor) {
	visitor.visit(*this);
}

ExpressionStatement::ExpressionStatement(
	const std::shared_ptr<Expression>& expression
	) : expression(expression) {}

void ExpressionStatement::accept(Visitor& visitor) {
	visitor.visit(*this);
}
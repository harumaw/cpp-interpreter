#include "visitor.hpp"

Declaration::Declarator::Declarator(
	const std::string& name
	) : name(name) {}

void Declaration::NoPtrDeclarator::accept(Visitor& visitor) {
	visitor.visit(*this);
}

void Declaration::PtrDeclarator::accept(Visitor& visitor) {
	visitor.visit(*this);
}

Declaration::InitDeclarator::InitDeclarator(
	const std::shared_ptr<Declarator>& declarator,
	const std::shared_ptr<Expression>& initializer
	) : declarator(declarator), initializer(initializer) {}

void Declaration::InitDeclarator::accept(Visitor& visitor) {
	visitor.visit(*this);
}

VarDeclaration::VarDeclaration(
	const std::string& type,
	const std::vector<std::shared_ptr<InitDeclarator>>& declarator_list
	) : type(type), declarator_list(declarator_list) {}

void VarDeclaration::accept(Visitor& visitor) {
	visitor.visit(*this);
}

FuncDeclaration::FuncDeclaration(
	const std::string& type,
	const std::shared_ptr<Declarator>& declarator,
	const std::vector<std::shared_ptr<ParameterDeclaration>>& args,
	const std::shared_ptr<CompoundStatement>& body
	) : type(type), declarator(declarator), args(args), body(body) {}

void FuncDeclaration::accept(Visitor& visitor) {
	visitor.visit(*this);
}

ParameterDeclaration::ParameterDeclaration(
	const std::string& type,
	const std::shared_ptr<InitDeclarator>& init_declarator
	) : type(type), init_declarator(init_declarator) {}

void ParameterDeclaration::accept(Visitor& visitor) {
	visitor.visit(*this);
}
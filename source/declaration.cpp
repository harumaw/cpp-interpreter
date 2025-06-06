#include "visitor.hpp"

Declaration::Declarator::Declarator(
	const std::string& name
	) : name(name) {}

void Declaration::SimpleDeclarator::accept(Visitor& visitor) {
	visitor.visit(*this);
}

Declaration::PtrDeclarator::PtrDeclarator(const std::shared_ptr<Declarator>& inner)
        : Declarator(inner->name), inner(inner) {}

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
	bool is_const,
	const std::string& type,
	const std::vector<std::shared_ptr<InitDeclarator>>& declarator_list
	) : is_const(is_const), type(type), declarator_list(declarator_list) {}

void VarDeclaration::accept(Visitor& visitor) {
	visitor.visit(*this);
}

FuncDeclaration::FuncDeclaration(
	bool is_const,
	const std::string& type,
	const std::shared_ptr<Declarator>& declarator,
	bool is_readonly,
	const std::vector<std::shared_ptr<ParameterDeclaration>>& args,
	const std::shared_ptr<CompoundStatement>& body
	) : is_const(is_const), type(type), declarator(declarator), is_readonly(is_readonly), args(args), body(body) {}

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

StructDeclaration:: StructDeclaration 
    (const std::string& name, 
                      const std::vector<std::shared_ptr<Declaration>>& members)
        : name(name), members(members) {}

void StructDeclaration::accept(Visitor& visitor) {
        visitor.visit(*this);
}


ArrayDeclaration::ArrayDeclaration(const std::string& type,
                                   const std::string& name,
                                   const std::shared_ptr<Expression>& size,
								const std::vector<std::shared_ptr<Expression>>& initializer_list)
	: type(type), name(name), size(size), initializer_list(initializer_list) {}

void ArrayDeclaration::accept(Visitor& visitor) {
	visitor.visit(*this);
}

NameSpaceDeclaration::NameSpaceDeclaration(const std::string& name,
								const std::vector<std::shared_ptr<Declaration>>& declarations)
	: name(name), declarations(declarations) {}
void NameSpaceDeclaration::accept(Visitor& visitor) {
	visitor.visit(*this);
}


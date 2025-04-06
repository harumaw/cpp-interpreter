#pragma once

#include <string>
#include <vector>
#include <memory>

#include "ast.hpp"

struct CompoundStatement;

struct Declaration::Declarator {
	std::string name;
	Declarator(const std::string&);

	virtual ~Declarator() = default;
	virtual void accept(Visitor&) = 0;
};

struct Declaration::NoPtrDeclarator : public Declaration::Declarator{
	using Declarator::Declarator;

	void accept(Visitor&) override;
};

struct Declaration::PtrDeclarator : public Declaration::Declarator{
	using Declarator::Declarator;

	void accept(Visitor&) override;
};

struct Declaration::InitDeclarator {
	std::shared_ptr<Declarator> declarator;
	std::shared_ptr<Expression> initializer;

	InitDeclarator(const std::shared_ptr<Declarator>&, const std::shared_ptr<Expression>&);
	void accept(Visitor&);
};

struct VarDeclaration: public Declaration {
	std::string type;
	std::vector<std::shared_ptr<InitDeclarator>> declarator_list;

	VarDeclaration(const std::string&, const std::vector<std::shared_ptr<InitDeclarator>>&);
	void accept(Visitor&) override;
};

struct ParameterDeclaration: public Declaration {
	std::string type;
	std::shared_ptr<InitDeclarator> init_declarator;

	ParameterDeclaration(const std::string&, const std::shared_ptr<InitDeclarator>&);
	void accept(Visitor&) override;
};

struct FuncDeclaration: public Declaration {
	std::string type;
	std::shared_ptr<Declarator> declarator;
	std::vector<std::shared_ptr<ParameterDeclaration>> args;
	std::shared_ptr<CompoundStatement> body;

	FuncDeclaration(const std::string&,
					const std::shared_ptr<Declarator>&,
					const std::vector<std::shared_ptr<ParameterDeclaration>>&,
					const std::shared_ptr<CompoundStatement>&
					);

	void accept(Visitor&) override;
};
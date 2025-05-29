#pragma once

#include <string>
#include <vector>
#include <memory>

#include "ast.hpp"

struct CompoundStatement;


struct Declaration:: Declarator {
	std::string name;
	Declarator(const std::string&);

	virtual ~Declarator() = default;
	virtual void accept(Visitor&) = 0;
};

struct Declaration::SimpleDeclarator : public Declaration::Declarator{
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
	bool is_const = false; // std::vector<std::string> modifiers
	std::string type;
	std::vector<std::shared_ptr<InitDeclarator>> declarator_list;

	VarDeclaration(bool is_const, const std::string&, const std::vector<std::shared_ptr<InitDeclarator>>&);
	void accept(Visitor&) override;
};

struct ParameterDeclaration: public Declaration {
	std::string type;
	std::shared_ptr<InitDeclarator> init_declarator;

	ParameterDeclaration(const std::string&, const std::shared_ptr<InitDeclarator>&);
	void accept(Visitor&) override;
};

struct FuncDeclaration: public Declaration {
	bool is_const = false; //std::vector<std::string> modifiers
	std::string type;
	std::shared_ptr<Declarator> declarator;
	bool is_readonly = false;
	std::vector<std::shared_ptr<ParameterDeclaration>> args;
	std::shared_ptr<CompoundStatement> body;

	FuncDeclaration(
					bool is_const,
					const std::string&,
					const std::shared_ptr<Declarator>&,
					bool is_readonly,
					const std::vector<std::shared_ptr<ParameterDeclaration>>&,
					const std::shared_ptr<CompoundStatement>&
					);

	void accept(Visitor&) override;
};

struct StructDeclaration: public Declaration {
	std::string name;
	std::vector<std::shared_ptr<Declaration>> members;
	StructDeclaration(const std::string& name, 
						const std::vector<std::shared_ptr<Declaration>>& members);

	void accept(Visitor&) override;

};

struct ArrayDeclaration: public Declaration { 
 	std::string type;
	std::string name;
	std::shared_ptr<Expression> size;
	std::vector<std::shared_ptr<Expression>> initializer_list;


	ArrayDeclaration(const std::string& type, const std::string& name, const std::shared_ptr<Expression>& size, 
					const std::vector<std::shared_ptr<Expression>>& initializer_list);

    void accept(Visitor &visitor) override;
};

struct NameSpaceDeclaration : public Declaration{
	std::string name;
	std::vector<std::shared_ptr<Declaration>> declarations;

	NameSpaceDeclaration(const std::string& name, const std::vector<std::shared_ptr<Declaration>>& declarations);

	void accept(Visitor&) override;
};






using node = std::shared_ptr<TranslationUnit>;
using declaration = std::shared_ptr<Declaration>;
using func_declaration = std::shared_ptr<FuncDeclaration>;
using parameter_declaration = std::shared_ptr<ParameterDeclaration>;
using var_declaration = std::shared_ptr<VarDeclaration>;
using init_declarator = std::shared_ptr<Declaration::InitDeclarator>;
using declarator = std::shared_ptr<Declaration::Declarator>;
using struct_declaration = std::shared_ptr<StructDeclaration>;
using array_declaration = std::shared_ptr<ArrayDeclaration>;
using name_space_declaration = std::shared_ptr<NameSpaceDeclaration>;
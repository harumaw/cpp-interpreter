#pragma once

#include <string>
#include <vector>
#include <memory>

#include "ast.hpp"

struct VarDeclaration;

using statementseq= std::vector<std::shared_ptr<Statement>>;

struct CompoundStatement: public Statement {
	statementseq statements;

	CompoundStatement(const statementseq&);
	void accept(Visitor&) override;
};

struct ConditionalStatement : public Statement {
    ConditionalStatement(
        const std::pair<std::shared_ptr<Expression>, std::shared_ptr<Statement>>& if_branch,
        const std::shared_ptr<Statement>& else_branch
    );
    std::pair<std::shared_ptr<Expression>, std::shared_ptr<Statement>> if_branch;
    std::shared_ptr<Statement> else_branch;

    void accept(Visitor& visitor) override; 
};
struct LoopStatement: public Statement {
	virtual ~LoopStatement() = default;
	virtual void accept(Visitor&) override = 0;
};

struct WhileStatement: public LoopStatement {
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Statement> statement;

	WhileStatement(
		const std::shared_ptr<Expression>&,
		const std::shared_ptr<Statement>&
	);
	void accept(Visitor&) override;
};


struct ForStatement : public LoopStatement {
    std::shared_ptr<ASTNode> initialization; 
    std::shared_ptr<Expression> condition;   
    std::shared_ptr<Expression> increment;   
    std::shared_ptr<Statement> body;         

	ForStatement(
        std::shared_ptr<ASTNode> initialization,
        std::shared_ptr<Expression> condition,
        std::shared_ptr<Expression> increment,
        std::shared_ptr<Statement> body
    );

    void accept(Visitor& visitor) override;
       
};

struct JumpStatement: public Statement {
	virtual ~JumpStatement() = default;
	virtual void accept(Visitor&) override = 0;
};

struct ReturnStatement: public JumpStatement {
	std::shared_ptr<Expression> expression;

	ReturnStatement(const std::shared_ptr<Expression>&);
	void accept(Visitor&) override;
};

struct BreakStatement: public JumpStatement {
	void accept(Visitor&) override;
};

struct ContinueStatement: public JumpStatement {
	void accept(Visitor&) override;
};

struct DeclarationStatement: public Statement {
	std::shared_ptr<Declaration> declaration;

	DeclarationStatement(const std::shared_ptr<Declaration>&);
	void accept(Visitor&) override;
};

struct ExpressionStatement: public Statement {
	std::shared_ptr<Expression> expression;

	ExpressionStatement(const std::shared_ptr<Expression>&);
	void accept(Visitor&) override;
};

struct DoWhileStatement : public LoopStatement{
	std::shared_ptr<Statement> statement;
	std::shared_ptr<Expression> condition;
	DoWhileStatement(
		const std::shared_ptr<Statement>&,
		const std::shared_ptr<Expression>&
	);
	void accept(Visitor&) override;
};

struct StaticAssertStatement : public Statement {
	std::shared_ptr<Expression> condition;
	std::string msg;

	StaticAssertStatement(const std::shared_ptr<Expression>&, const std::string&);

	void accept(Visitor&) override;
};


using statement = std::shared_ptr<Statement>;
using compound_statement = std::shared_ptr<CompoundStatement>;
using conditional_statement = std::shared_ptr<ConditionalStatement>;
using loop_statement = std::shared_ptr<LoopStatement>;
using while_statement = std::shared_ptr<WhileStatement>;
using for_statement = std::shared_ptr<ForStatement>;
using jump_statement = std::shared_ptr<JumpStatement>;
using break_statement = std::shared_ptr<BreakStatement>;
using continue_statement = std::shared_ptr<ContinueStatement>;
using return_statement = std::shared_ptr<ReturnStatement>;
using declaration_statement = std::shared_ptr<DeclarationStatement>;
using expression_statement = std::shared_ptr<ExpressionStatement>;
using do_while_statement = std::shared_ptr<DoWhileStatement>;
using stat_assert = std::shared_ptr<StaticAssertStatement>;


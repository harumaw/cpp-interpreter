#pragma once

#include <vector>
#include <memory>


class Visitor;

struct ASTNode {
	virtual ~ASTNode() = default;
	virtual void accept(Visitor&) = 0;
};

struct Statement: public ASTNode {
	virtual ~Statement() = default;
	virtual void accept(Visitor&) override = 0;
};

struct Declaration: public ASTNode {

	struct Declarator;
	struct SimpleDeclarator;
	struct PtrDeclarator;

	struct InitDeclarator;

	virtual ~Declaration() = default;
	virtual void accept(Visitor&) override = 0;
};

struct Expression: public ASTNode {
	virtual ~Expression() = default;
	virtual void accept(Visitor&) override = 0;
};

using DeclarationSeq = std::vector<std::shared_ptr<Declaration>>;

struct RootNode : public ASTNode {
    std::vector<std::shared_ptr<ASTNode>> declarations;

    RootNode(const std::vector<std::shared_ptr<ASTNode>>& declarations);
    void accept(Visitor& visitor) override;
};
#include "ast.hpp"
#include "visitor.hpp"

RootNode::RootNode(const std::vector<std::shared_ptr<ASTNode>>& declarations)
    : declarations(declarations) {}

void RootNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}
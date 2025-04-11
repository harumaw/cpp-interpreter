#include "ast.hpp"
#include "visitor.hpp"

TranslationUnit::TranslationUnit(const std::vector<std::shared_ptr<ASTNode>>& declarations)
    : declarations(declarations) {}

void TranslationUnit::accept(Visitor& visitor) {
    visitor.visit(*this);
}
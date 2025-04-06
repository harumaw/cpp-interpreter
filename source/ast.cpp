#include "visitor.hpp"

TranslationUnit::TranslationUnit(
	const DeclarationSeq& declarations
	) : declarations(declarations) {}

void TranslationUnit::accept(Visitor& visitor) {
	visitor.visit(*this);
}
#include "executer.hpp"

Execute::Execute() { table.reset(); }  


void Execute::execute(TranslationUnit& unit){
    table = std::make_shared<SymbolTable>(nullptr, nullptr);
    for (auto& node : unit.get_nodes()) {
        node->accept(*this);
    }
}

void Execute::visit(TranslationUnit& unit) {
    for (auto& node : unit.get_nodes()) {
        node->accept(*this);
    }
}   
void Execute::visit(Declaration::PtrDeclarator&) {}
void Execute::visit(Declaration::SimpleDeclarator&) {}

void Execute::visit(Declaration::InitDeclarator& init) {
    if (init.initializer) {
        init.initializer->accept(*this);
    }
}

void Execute::visit(VarDeclaration& node) {
    for (auto& initDecl : node.declarator_list) {
        Value val;

        if (initDecl->initializer) {
            // если есть явная инициализация — вычисляем её
            initDecl->initializer->accept(*this);
            val = current_value;
        } else {
            // дефолт по типу
            if (node.type == "int") {
                val = Value::makeInt(0);
            }
            else if (node.type == "float") {
                val = Value::makeFloat(0.0f);
            }
            else if (node.type == "char") {
                val = Value::makeChar(u'\0');
            }
            else if (node.type == "bool") {
                val = Value::makeBool(false);
            }
            else if (table->match_struct(node.type).size() >= 0) {  // ?
                val = Value::makeStruct({});
            }
            else {
                val = Value::makeVoid();
            }
        }

        table->push_variable(initDecl->declarator->name, val);
    }
}



void Execute::visit(ParameterDeclaration&) {}

void Execute::visit(FuncDeclaration&) {}



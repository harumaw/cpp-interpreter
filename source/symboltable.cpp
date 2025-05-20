
#include "symboltable.hpp"


std::shared_ptr<SymbolTable> SymbolTable::get_prev_table(){
    return prev_table;
}

std::shared_ptr<SymbolTable> SymbolTable::create_new_table(){
    return std::make_shared<SymbolTable>()
}
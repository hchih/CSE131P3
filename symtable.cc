/*
 * Symbol table implementation
 *
 */
#include "symtable.h"
SymbolTable::SymbolTable(){
  push(new ScopedTable);
}
void SymbolTable::push(ScopedTable* scopedtab){
  tables.push_back(scopedtab);
}
void SymbolTable::pop(){
  tables.pop_back();
}
void ScopedTable::insert(Symbol &sym){
  char* name = sym.name;
  symbols.insert(std::pair<char*, Symbol>(name, sym));
}
void ScopedTable::remove(Symbol &sym){
  char* name = sym.name;
  symbols.erase(name);
}
Symbol* ScopedTable::find(const char* name){
  if (symbols.find(name) == symbols.end())
    return NULL;
  return &(symbols.find(name)->second);
}
bool MyStack::insideLoop(){
  if(stmtStack.size() > 0){
    return true;
  }
  return false;
}
bool MyStack::insideSwitch(){
  if(stmtStack.size() > 0){
    return true;
  }
  return false;
}





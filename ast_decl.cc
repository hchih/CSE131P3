/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symtable.h"        
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}

VarDecl::VarDecl(Identifier *n, Type *t, Expr *e) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
    typeq = NULL;
}
VarDecl::VarDecl(Identifier *n, TypeQualifier *tq, Expr *e) : Decl(n) {
    Assert(n != NULL && tq != NULL);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
    type = NULL;
}

VarDecl::VarDecl(Identifier *n, Type *t, TypeQualifier *tq, Expr *e) : Decl(n) {
    Assert(n != NULL && t != NULL && tq != NULL);
    (type=t)->SetParent(this);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
}
  
void VarDecl::PrintChildren(int indentLevel) { 
   if (typeq) typeq->Print(indentLevel+1);
   if (type) type->Print(indentLevel+1);
   if (id) id->Print(indentLevel+1);
   if (assignTo) assignTo->Print(indentLevel+1, "(initializer) ");
}

void VarDecl::Check(){
    char* name = this->GetIdentifier()->GetName();
    Symbol* sym = new Symbol(name, this, E_VarDecl);
    
    Symbol* found = symtab->tables[scopeLevel]->find(name);
    // Declaration already exists
    if( found != NULL){
        ReportError::DeclConflict(this, found->decl);
        symtab->tables[scopeLevel]->remove(*found); 
    }
    symtab->tables[scopeLevel]->insert(*sym);

    // Check initialization
    if(assignTo != NULL){
        assignTo->Check();
        if(assignTo->GetType() != type){
            if(assignTo->GetType() != Type::errorType){
                ReportError::InvalidInitialization(this->GetIdentifier(), type, assignTo->GetType());
            }
        }
    }
}

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
    returnTypeq = NULL;
}

FnDecl::FnDecl(Identifier *n, Type *r, TypeQualifier *rq, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r != NULL && rq != NULL&& d != NULL);
    (returnType=r)->SetParent(this);
    (returnTypeq=rq)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

void FnDecl::PrintChildren(int indentLevel) {
    if (returnType) returnType->Print(indentLevel+1, "(return type) ");
    if (id) id->Print(indentLevel+1);
    if (formals) formals->PrintAll(indentLevel+1, "(formals) ");
    if (body) body->Print(indentLevel+1, "(body) ");
}
void FnDecl::Check(){
    char* name = this->GetIdentifier()->GetName();
    Symbol* sym = new Symbol(name, this, E_FunctionDecl);
    
    Symbol* found = symtab->tables[scopeLevel]->find(name);

    if(found != NULL){
        ReportError::DeclConflict(this, found->decl);
        symtab->tables[scopeLevel]->remove(*found);
    }
    symtab->tables[scopeLevel]->insert(*sym);
    scopeLevel++;
    symtab->push(new ScopedTable);
    /* Inserting parameters */
    for(int i=0;i<formals->NumElements();i++){
        VarDecl* param = formals->Nth(i);
        char* name = param->GetIdentifier()->GetName();
        Symbol* sym = new Symbol(name, param, E_VarDecl);

        symtab->tables[scopeLevel]->insert(*sym);
    }
    if(body != NULL)
        body->Check(); // Return type !!
    scopeLevel--;
    symtab->pop();
}


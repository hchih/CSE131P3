/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
 #include <regex>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "symtable.h"

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}
void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}

FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
    printf("%g", value);
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
    printf("%s", value ? "true" : "false");
}

VarExpr::VarExpr(yyltype loc, Identifier *ident) : Expr(loc) {
    Assert(ident != NULL);
    this->id = ident;
}

void VarExpr::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}

void Operator::PrintChildren(int indentLevel) {
    printf("%s",tokenString);
}

bool Operator::IsOp(const char *op) const {
    return strcmp(tokenString, op) == 0;
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o) 
  : Expr(Join(l->GetLocation(), o->GetLocation())) {
    Assert(l != NULL && o != NULL);
    (left=l)->SetParent(this);
    (op=o)->SetParent(this);
}

void CompoundExpr::PrintChildren(int indentLevel) {
   if (left) left->Print(indentLevel+1);
   op->Print(indentLevel+1);
   if (right) right->Print(indentLevel+1);
}
   
ConditionalExpr::ConditionalExpr(Expr *c, Expr *t, Expr *f)
  : Expr(Join(c->GetLocation(), f->GetLocation())) {
    Assert(c != NULL && t != NULL && f != NULL);
    (cond=c)->SetParent(this);
    (trueExpr=t)->SetParent(this);
    (falseExpr=f)->SetParent(this);
}

void ConditionalExpr::PrintChildren(int indentLevel) {
    cond->Print(indentLevel+1, "(cond) ");
    trueExpr->Print(indentLevel+1, "(true) ");
    falseExpr->Print(indentLevel+1, "(false) ");
}
void EqualityExpr::Check(){

    this->SetType(Type::boolType);
    left->Check();
    
    if(left->GetType() == Type::errorType){
        this->SetType(Type::errorType);
        return;
    }

    right->Check();

    if(right->GetType() == Type::errorType){
        this->SetType(Type::errorType);
        return;
    }
    if(left->GetType() != right->GetType()){
        ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
        this->SetType(Type::errorType);
    }

}
void RelationalExpr::Check(){
    this->SetType(Type::boolType);
    left->Check();
    
    if(left->GetType() == Type::errorType){
        this->SetType(Type::errorType);
        return;
    }

    right->Check();

    if(right->GetType() == Type::errorType){
        this->SetType(Type::errorType);
        return;
    }
    if(left->GetType() != right->GetType()){
        ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
        this->SetType(Type::errorType);
    }
    if(left->GetType() == Type::boolType){
        ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
        this->SetType(Type::errorType);
    }
}
void LogicalExpr::Check(){

    this->SetType(Type::boolType);
    left->Check();
    
    if(left->GetType() == Type::errorType){
        this->SetType(Type::errorType);
        return;
    }

    right->Check();

    if(right->GetType() == Type::errorType){
        this->SetType(Type::errorType);
        return;
    }
    if(left->GetType() != right->GetType()){
        ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
        this->SetType(Type::errorType);
    }
    if(left->GetType() != Type::boolType){
        ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
        this->SetType(Type::errorType);
    }

}
void AssignExpr::Check(){
    left->Check();
    
    if(left->GetType() == Type::errorType){
        this->SetType(Type::errorType);
        return;
    }

    right->Check();
    if(right->GetType() == Type::errorType){
        this->SetType(Type::errorType);
        return;
    }
    if(left->GetType() != right->GetType()){
        ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
        this->SetType(Type::errorType);
    }
    // std::cerr<<"not segfault yet============"<<std::endl;
    char* tokenString = op->getTokenString();

    if((tokenString[0]) != '='){
        if(left->GetType() == Type::boolType){
            ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
            this->SetType(Type::errorType);
        }
    }
    // std::cerr<<"not segfault yet============="<<std::endl;
}
void ArithmeticExpr::Check(){
    if(left != NULL){
        left->Check();
        
        if(left->GetType() == Type::errorType){
            this->SetType(Type::errorType);
            return;
        }

        if(left->GetType() == Type::boolType){
            this->SetType(Type::errorType);
            if(right == NULL)
                ReportError::IncompatibleOperand(op, left->GetType());
            else{
                right->Check();
                ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
            }
            return;
        }
    }
    if(right != NULL){
        right->Check();

        if(right->GetType() == Type::errorType){
            this->SetType(Type::errorType);
            return;
        }
        if(right->GetType() == Type::boolType){
            this->SetType(Type::errorType);
            if(left == NULL)
                ReportError::IncompatibleOperand(op, right->GetType());
            else{
                ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
            }
            return;
        }
    }
    if(left != NULL && right != NULL){
        if(left->GetType() != right->GetType()){
            ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
            this->SetType(Type::errorType);
        }
        
        if(left->GetType() == Type::boolType){
            ReportError::IncompatibleOperands(op, left->GetType(), right->GetType());
            this->SetType(Type::errorType);
        }
    }
    
}
void VarExpr::Check(){ 
    int currentScope = scopeLevel;
    Type* type = NULL;

    while(currentScope >= 0){
        Symbol* symbol = symtab->tables[currentScope]->find(id->GetName());
        if(symbol != NULL){
            VarDecl* vardecl = dynamic_cast<VarDecl*>(symbol->decl);
            type = vardecl->GetType();
            break;
        }
        currentScope--;
    }
    
    if(type == NULL){
        ReportError::IdentifierNotDeclared(id, LookingForVariable);
        Expr::SetType(Type::errorType);
    } 
    else{
        Expr::SetType(type); 
    }
}
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

void ArrayAccess::PrintChildren(int indentLevel) {
    base->Print(indentLevel+1);
    subscript->Print(indentLevel+1, "(subscript) ");
}
void ArrayAccess::Check(){
    VarExpr* array = dynamic_cast<VarExpr*>(base);
    array->Check();

    if(dynamic_cast<ArrayType*>(array->GetType()) == NULL){
        ReportError::NotAnArray(array->GetIdentifier());
        Expr::SetType(Type::errorType);
    }
    else{
        Expr::SetType(dynamic_cast<ArrayType*>(array->GetType())->GetElemType());
    }
}  
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}
void FieldAccess::PrintChildren(int indentLevel) {
    if (base) base->Print(indentLevel+1);
    field->Print(indentLevel+1);
}
void FieldAccess::Check(){
    base->Check();
    Type* type = base->GetType();
    std::string fieldname = field->GetName();

    std::regex e ("[A-V a-v 0-9]+");
    // Not vector type
    if( type != Type::vec2Type && type != Type::vec3Type && type != Type::vec4Type){
        ReportError::InaccessibleSwizzle(field,base);
        
    }
    // Invalid field name
    if(std::regex_match(fieldname, e)){
        ReportError::InvalidSwizzle(field,base);
    }
    // fieldname contains x,y,z,w
    else{
        if(fieldname.length() > 4){
            ReportError::OversizedVector(field,base);
            return;
        }
        if(type == Type::vec2Type){
            if( fieldname.find("z") != string::npos || fieldname.find("w") != string::npos){
                ReportError::SwizzleOutOfBound(field,base);
            }
        }
        else if(type == Type::vec3Type){
            if( fieldname.find("w") != string::npos ){
                ReportError::SwizzleOutOfBound(field,base);
            }
        }
    }
    
}
Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}
void Call::PrintChildren(int indentLevel) {
   if (base) base->Print(indentLevel+1);
   if (field) field->Print(indentLevel+1);
   if (actuals) actuals->PrintAll(indentLevel+1, "(actuals) ");
}
void Call::Check(){
    int currentScope = scopeLevel;
    Type* type = NULL;
    List<VarDecl*> *formals;
    while(currentScope >= 0){
        Symbol* symbol = symtab->tables[currentScope]->find(field->GetName());
        if(symbol != NULL){
            if(symbol->kind == E_FunctionDecl){
                FnDecl* fndecl = dynamic_cast<FnDecl*>(symbol->decl);
                type = fndecl->GetType();
                formals = fndecl->GetFormals();
                break;
            }  
            else{
                ReportError::NotAFunction(field);
                // VarDecl* vardecl = dynamic_cast<VarDecl*>(symbol->decl);
                // Expr::SetType(vardecl->GetType());
                return;
            }
        }
        currentScope--;
    }
    if(type == NULL){
        ReportError::IdentifierNotDeclared(field, LookingForVariable);
        Expr::SetType(Type::errorType);
    } 
    else{
        // return type
        Expr::SetType(type); 
        if(actuals->NumElements() < formals->NumElements()){
            ReportError::LessFormals(field, formals->NumElements(), actuals->NumElements());
        }
        else if (actuals->NumElements() > formals->NumElements()){
            ReportError::ExtraFormals(field, formals->NumElements(), actuals->NumElements());
        }
        else{
            for(int i=0; i< actuals->NumElements();i++){
                actuals->Nth(i)->Check();

                if(actuals->Nth(i)->GetType() != formals->Nth(i)->GetType()){
                    ReportError::FormalsTypeMismatch(field, i+1, 
                        formals->Nth(i)->GetType(), actuals->Nth(i)->GetType());
                    return;
                }
            }
        }
    }





}

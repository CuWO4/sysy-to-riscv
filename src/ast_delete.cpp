#include "../include/ast.h"

namespace ast {

BinaryExpr::~BinaryExpr() {
    if (lv != nullptr) delete lv;
    if (rv != nullptr) delete rv;
}

UnaryExpr::~UnaryExpr() {
    if (lv != nullptr) delete lv;
}

Id::~Id() {
    if (lit != nullptr) delete lit;
}

ExprStmt::~ExprStmt() {
    if (expr != nullptr) delete expr;
}

VarDef::~VarDef() {
    if (id != nullptr) delete id;
    if (init != nullptr) delete init;
}

VarDecl::~VarDecl() {
    if (type != nullptr) delete type;
    for (auto var_def : var_defs) {
        if (var_def != nullptr) delete var_def;
    }
}

Assign::~Assign() {
    if (id != nullptr) delete id;
    if (rval != nullptr) delete rval;
}

Return::~Return() {
    if (ret_val != nullptr) delete ret_val;
}

If::~If() {
    if (cond != nullptr) delete cond;
    if (then_stmt != nullptr) delete then_stmt;
    if (else_stmt != nullptr) delete else_stmt;
}

Block::~Block() {
    for (auto stmt : stmts) {
        if (stmt != nullptr) delete stmt;
    }
}

FuncDef::~FuncDef() {
    if (func_type != nullptr) delete func_type;
    if (id != nullptr) delete id;
    if (block != nullptr) delete block;
}

CompUnit::~CompUnit() {
    if (func_def != nullptr) delete func_def;
}

}
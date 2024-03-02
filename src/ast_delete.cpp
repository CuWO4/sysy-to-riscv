#include "../include/ast.h"

ast::BinaryExpr::~BinaryExpr() {
    if (lv != nullptr) delete lv;
    if (rv != nullptr) delete rv;
}

ast::UnaryExpr::~UnaryExpr() {
    if (lv != nullptr) delete lv;
}

ast::Id::~Id() {
    if (lit != nullptr) delete lit;
}

ast::ExprStmt::~ExprStmt() {
    if (expr != nullptr) delete expr;
}

ast::VarDef::~VarDef() {
    if (id != nullptr) delete id;
    if (init != nullptr) delete init;
}

ast::VarDecl::~VarDecl() {
    if (type != nullptr) delete type;
    for (auto var_def : var_defs) {
        if (var_def != nullptr) delete var_def;
    }
}

ast::Assign::~Assign() {
    if (id != nullptr) delete id;
    if (rval != nullptr) delete rval;
}

ast::Return::~Return() {
    if (ret_val != nullptr) delete ret_val;
}

ast::If::~If() {
    if (cond != nullptr) delete cond;
    if (then_stmt != nullptr) delete then_stmt;
    if (else_stmt != nullptr) delete else_stmt;
}

ast::Block::~Block() {
    for (auto stmt : stmts) {
        if (stmt != nullptr) delete stmt;
    }
}

ast::FuncDef::~FuncDef() {
    if (func_type != nullptr) delete func_type;
    if (id != nullptr) delete id;
    if (block != nullptr) delete block;
}

ast::CompUnit::~CompUnit() {
    if (func_def != nullptr) delete func_def;
}
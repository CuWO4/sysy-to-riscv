#include "../include/ast.h"
#include "../include/koopa.h"
#include "../include/trans.h"
#include "../include/nesting_info.h"
#include "../include/def.h"
#include "../include/value_saver.h"

#include <string>

namespace ast {

koopa_trans::Blocks *BinaryExpr::to_koopa(ValueSaver &value_saver) const {
    auto res = new koopa_trans::Blocks;

    auto lv_stmts = lv->to_koopa(value_saver);
    auto rv_stmts = rv->to_koopa(value_saver);

    *res += *lv_stmts;
    *res += *rv_stmts;

    auto generator = [&] (koopa::op::Op op, koopa::Value *elv, koopa::Value *erv) {
        if (elv->is_const && erv->is_const) {
            res->active_stmts.clear();

            res->last_val = value_saver.new_const(
                koopa::op::op_func[op](elv->val, erv->val)
            );

            return;
        }

        res->last_val = value_saver.new_id(
            new koopa::Int,
            new std::string('%' + std::to_string(tmp_var_count++)),
            new NestingInfo(false)
        );

        *res += new koopa::SymbolDef(
            static_cast<koopa::Id *>(res->last_val),
            new koopa::Expr(
                op, elv, erv
            )
        );
    };

    switch (op) {
        case op::LOGIC_AND: {
            generator(koopa::op::NE, lv_stmts->last_val, value_saver.new_const(0));
            auto tmpa = res->last_val;
            generator(koopa::op::NE, rv_stmts->last_val, value_saver.new_const(0));
            auto tmpb = res->last_val;
            generator(koopa::op::AND, tmpa, tmpb);
            break;
        }
        case op::LOGIC_OR: {
            generator(koopa::op::NE, lv_stmts->last_val, value_saver.new_const(0));
            auto tmpa = res->last_val;
            generator(koopa::op::NE, rv_stmts->last_val, value_saver.new_const(0));
            auto tmpb = res->last_val;
            generator(koopa::op::OR, tmpa, tmpb);
            break;
        }
        case op::EQ: generator(koopa::op::EQ, lv_stmts->last_val, rv_stmts->last_val); break;
        case op::NEQ: generator(koopa::op::NE, lv_stmts->last_val, rv_stmts->last_val); break;
        case op::LT: generator(koopa::op::LT, lv_stmts->last_val, rv_stmts->last_val); break;
        case op::GT: generator(koopa::op::GT, lv_stmts->last_val, rv_stmts->last_val); break;
        case op::LEQ: generator(koopa::op::LE, lv_stmts->last_val, rv_stmts->last_val); break;
        case op::GEQ: generator(koopa::op::GE, lv_stmts->last_val, rv_stmts->last_val); break;
        case op::ADD: generator(koopa::op::ADD, lv_stmts->last_val, rv_stmts->last_val); break;
        case op::SUB: generator(koopa::op::SUB, lv_stmts->last_val, rv_stmts->last_val); break;
        case op::MUL: generator(koopa::op::MUL, lv_stmts->last_val, rv_stmts->last_val); break;
        case op::DIV: generator(koopa::op::DIV, lv_stmts->last_val, rv_stmts->last_val); break;
        case op::MOD: generator(koopa::op::MOD, lv_stmts->last_val, rv_stmts->last_val); break;
    }

    delete lv_stmts;
    delete rv_stmts;

    return res;
}

koopa_trans::Blocks *UnaryExpr::to_koopa(ValueSaver &value_saver) const {
    auto lv_stmts = lv->to_koopa(value_saver);

    auto generator = [&](koopa::op::Op op) {
        if (lv_stmts->last_val->is_const) {
            lv_stmts->active_stmts.clear();

            lv_stmts->last_val = value_saver.new_const(koopa::op::op_func[op](0, lv_stmts->last_val->val));

            return;
        }

        if (op == koopa::op::ADD) return;

        koopa::NotEndStmt *new_stmt;
        koopa::Id *new_id;

        new_id = value_saver.new_id(
            new koopa::Int,
            new std::string('%' + std::to_string(tmp_var_count++)),
            new NestingInfo(false)
        );

        new_stmt = new koopa::SymbolDef(
            new_id,
            new koopa::Expr(
                op,
                value_saver.new_const(0),
                lv_stmts->last_val
            )
        );

        *lv_stmts += new_stmt;
        lv_stmts->last_val = new_id;
    };

    switch (op) {
        case op::POS:

            generator(koopa::op::ADD);

            break;

        case op::NOT:

            generator(koopa::op::EQ);

            break;

        case op::NEG:

            generator(koopa::op::SUB);
    }

    return lv_stmts;
}

koopa_trans::Blocks *Number::to_koopa(ValueSaver &value_saver) const {
    auto res = new koopa_trans::Blocks;
    res->last_val = value_saver.new_const(val);
    return res;
}

koopa_trans::Blocks *ExprStmt::to_koopa(ValueSaver &value_saver) const {
    return expr->to_koopa(value_saver);
}

koopa_trans::Blocks *Id::to_koopa(ValueSaver &value_saver) const {
    auto res = new koopa_trans::Blocks;

    auto id_koopa = value_saver.get_id('@' + *lit, nesting_info);

    if (id_koopa == nullptr) {
        throw "undeclared identifier `" + *lit + '`';
    }

    if (id_koopa->is_const) {

        res->last_val = value_saver.new_const(id_koopa->val);
        return res;

    }
    else {

        auto new_id = value_saver.new_id(
            static_cast<koopa::Pointer *>(id_koopa->type)->pointed_type, //!? re-free bug
            new std::string('%' + std::to_string(tmp_var_count++)),
            new NestingInfo(false)
        );

        *res += new koopa::SymbolDef(
            new_id,
            new koopa::Load(id_koopa)
        );

        res->last_val = new_id;

        return res;

    }
}

koopa_trans::Blocks *VarDecl::to_koopa(ValueSaver &value_saver) const {
    auto stmts = new koopa_trans::Blocks();

    if (is_const) {

        for (auto var_def : var_defs) {
            if (value_saver.is_id_declared('@' + *var_def->id->lit, var_def->id->nesting_info)) {
                throw '`' + *var_def->id->lit + "` redefined";
            }

            if (!var_def->has_init) {
                throw "no initiator for const variable `" + *var_def->id->lit + '`';
            }

            auto rval_koopa = var_def->init->to_koopa(value_saver);

            if (!rval_koopa->last_val->is_const) {
                throw "initiating const variable `" + *var_def->id->lit + "` with a non-const value";
            }

            value_saver.new_id(
                type->to_koopa(value_saver),
                new std::string('@' + *var_def->id->lit),
                var_def->id->nesting_info,
                true,
                rval_koopa->last_val->val
            );

            //! BUG refree memory
            // for (auto stmt : stmts->stmts) if (stmt != nullptr) delete stmt;
            // delete stmts;
        }

    }

    else {

        for (auto var_def : var_defs) {
            if (value_saver.is_id_declared('@' + *var_def->id->lit, var_def->id->nesting_info)) {
                throw '`' + *var_def->id->lit + "` redefined";
            }

            *stmts += new koopa::SymbolDef(
                value_saver.new_id(
                    new koopa::Pointer(
                        type->to_koopa(value_saver)
                    ),
                    new std::string('@' + *var_def->id->lit),
                    var_def->id->nesting_info
                ),
                new koopa::MemoryDecl(type->to_koopa(value_saver))
            );

            if (var_def->has_init) {

                auto rval_koopa = var_def->init->to_koopa(value_saver);

                *stmts += *rval_koopa;

                *stmts += new koopa::StoreValue(
                    rval_koopa->last_val,
                    value_saver.get_id('@' + *var_def->id->lit, var_def->id->nesting_info)
                );
        }
        }

    }

    return stmts;
}

koopa_trans::Blocks *Assign::to_koopa(ValueSaver &value_saver) const {
    auto res = rval->to_koopa(value_saver);

    auto id_koopa = value_saver.get_id('@' + *id->lit, id->nesting_info);

    if (id_koopa == nullptr) {
        throw "undeclared identifier `" + *id->lit + '`';
    }
    if (id_koopa->is_const) {
        throw "assigning to a const identifier `" + *id->lit + '`';
    }

    *res += new koopa::StoreValue(
        res->last_val,
        value_saver.get_id('@' + *id->lit, id->nesting_info)
    );

    return res;
}

koopa_trans::Blocks *Return::to_koopa(ValueSaver &value_saver) const {
    if (!has_return_val) {
        auto res = new koopa_trans::Blocks();
        *res += new koopa::Return();
        return res;
    }

    auto res = ret_val->to_koopa(value_saver);

    *res += new koopa::Return( res->last_val );

    return res;
}

koopa_trans::Blocks *If::to_koopa(ValueSaver &value_saver) const {
    auto res = new koopa_trans::Blocks;



    return res;
}

koopa_trans::Blocks *Block::to_koopa(ValueSaver &value_saver) const {
    auto res = new koopa_trans::Blocks;

    for (auto stmt : stmts) {
        *res += *stmt->to_koopa(value_saver);
    }

    //TODO trim redundant statements after `ret` `br` and `jump`, notice to remove the preds & succs

    return res;
}

koopa::Type *Int::to_koopa(ValueSaver &value_saver) const {
    return new koopa::Int;
}

koopa::FuncDef *FuncDef::to_koopa(ValueSaver &value_saver) const {
    auto func_id = new std::string("");
    *func_id += "@" + *id;
    return new koopa::FuncDef(
        value_saver.new_id(
            new koopa::FuncType(
                std::vector<koopa::Type *>(),
                func_type->to_koopa(value_saver)
            ),
            func_id,
            new NestingInfo(false) /* This does not affect scope, since we only name zero-nested variables as unsuffixed */
        ),
        std::vector<koopa::FuncParamDecl *>(),
        func_type->to_koopa(value_saver),
        block->to_koopa(value_saver)->to_raw_blocks()
    );
}

koopa::Program *CompUnit::to_koopa(ValueSaver &value_saver) const {
    return new koopa::Program(
        std::vector<koopa::GlobalStmt *> {
            func_def->to_koopa(value_saver)
        }
    );
}

}
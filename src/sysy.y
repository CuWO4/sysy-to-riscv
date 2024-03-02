%code requires {
    #include <iostream>
    #include <memory>
    #include <string>

    #include "../include/ast.h"
    #include "../include/nesting_info.h"
    #include "../include/trans.h"

    int yylex();
    void yyerror(ast::CompUnit *&ast, const char *s);
}

%{
    #include "../include/ast.h"

    int cur_nesting_level = 0;
    int cur_nesting_count[4096] = { 0 };
    NestingInfo *cur_nesting_info = nullptr;
%}

%parse-param    { ast::CompUnit *&ast }

%union {
    std::string    *str_val;
    int             int_val;

    ast::FuncDef   *ast_func_def_val;
    ast::Type      *ast_type_val;
    ast::Block     *ast_block_val;
    ast::Stmt      *ast_stmt_val;
    ast::Expr      *ast_expr_val;
    ast::VarDef    *ast_var_def_val;
    std::vector<ast::Stmt *>    *ast_stmt_vec_val;
    std::vector<ast::VarDef *>  *ast_var_def_vec_val;
	}

%token TK_INT TK_RETURN TK_CONST TK_IF TK_ELSE
%token <str_val> TK_IDENT
%token <int_val> TK_INT_CONST

%type	<ast_func_def_val> func_def
%type	<ast_type_val> type
%type	<ast_block_val> block
%type	<ast_stmt_val> block_item if_clause decl_stmt assign_stmt return_stmt expr_stmt stmt
%type	<ast_expr_val> expr
%type	<ast_var_def_val> var_def
%type   <ast_stmt_vec_val> block_items 
%type   <ast_var_def_vec_val> var_defs
%type   <int_val> number

%left TK_LOGIC_OR
%left TK_LOGIC_AND
%left TK_EQ TK_NEQ
%left '<' '>' TK_LEQ TK_GEQ
%left '+' '-'
%left '*' '/' '%'
%right PREC_UNARY_OP

// resolve the dangling-else by give a precedence 
%nonassoc PREC_IF
%nonassoc TK_ELSE

%%

comp_unit
    : func_def {
        ast = new ast::CompUnit($1);
    }
;

func_def
    : type TK_IDENT '(' ')' block {
        $$ = new ast::FuncDef(
            $1,
            $2,
            $5
        );
    }
;

type
    : TK_INT {
        $$ = new ast::Int();
    }
;

block
    : block_start block_items '}' {
        $$ = new ast::Block(*$2, cur_nesting_info);

        cur_nesting_level--;

        cur_nesting_info = cur_nesting_info->pa;
    }
;

block_start : '{' {
    auto new_nesting_info = new NestingInfo(cur_nesting_level, cur_nesting_count[cur_nesting_level], cur_nesting_info);
    cur_nesting_info = new_nesting_info;

    cur_nesting_count[cur_nesting_level]++;
    cur_nesting_level++;
}

block_items
    : block_items block_item {
        if ($2 != nullptr) $1->push_back($2);
        $$ = $1;
    }
    | {
        $$ = new std::vector<ast::Stmt *>{};
    }
;

block_item
    : stmt
    | error ';'     { yyerrok; }
;

stmt
    : if_clause
    | decl_stmt
    | assign_stmt
    | return_stmt
    | expr_stmt
    | block         { $$ = $1; }
    | ';'           { $$ = nullptr; }
;

if_clause
    : TK_IF '(' expr ')' stmt %prec PREC_IF {
        $$ =new ast::If($3, $5);
    }
    | TK_IF '(' expr ')' stmt TK_ELSE stmt {
        $$ = new ast::If($3, $5, $7);
    }
;

decl_stmt
    : type var_defs ';' {
        $$ = new ast::VarDecl($1, *$2);
    }
    | TK_CONST type var_defs ';' {
        $$ = new ast::VarDecl($2, *$3, true);
    }
;

var_defs
    : var_defs ',' var_def {
        $1->push_back($3);
        $$ = $1;
    }
    | var_def {
        $$ = new std::vector<ast::VarDef *>{ $1 };
    }
;

var_def
    : TK_IDENT '=' expr {
        $$ = new ast::VarDef(new ast::Id($1), $3);
    }
    | TK_IDENT {
        $$ = new ast::VarDef(new ast::Id($1));
    }
;

assign_stmt
    : TK_IDENT '=' expr ';' {
        $$ = new ast::Assign(new ast::Id($1), $3);
    }
;

return_stmt
    : TK_RETURN expr ';' {
        $$ = new ast::Return($2);
    }
    | TK_RETURN ';' {
        $$ = new ast::Return();
    }
;

expr_stmt
    : expr ';' {
        $$ = new ast::ExprStmt($1);
    }
;

expr
    : '(' expr ')'                  { $$ = $2; }
    | expr TK_LOGIC_OR expr         {
		$$ = new ast::BinaryExpr(ast::op::LOGIC_OR, $1, $3);
	}
    | expr TK_LOGIC_AND expr        {
		$$ = new ast::BinaryExpr(ast::op::LOGIC_AND, $1, $3);
	}
    | expr TK_EQ expr               {
		$$ = new ast::BinaryExpr(ast::op::EQ, $1, $3);
	}
    | expr TK_NEQ expr              {
		$$ = new ast::BinaryExpr(ast::op::NEQ, $1, $3);
	}
    | expr '<' expr                 {
		$$ = new ast::BinaryExpr(ast::op::LT, $1, $3);
	}
    | expr '>' expr                 {
		$$ = new ast::BinaryExpr(ast::op::GT, $1, $3);
	}
    | expr TK_LEQ expr              {
		$$ = new ast::BinaryExpr(ast::op::LEQ, $1, $3);
	}
    | expr TK_GEQ expr              {
		$$ = new ast::BinaryExpr(ast::op::GEQ, $1, $3);
	}
    | expr '+' expr                 {
		$$ = new ast::BinaryExpr(ast::op::ADD, $1, $3);
	}
    | expr '-' expr                 {
		$$ = new ast::BinaryExpr(ast::op::SUB, $1, $3);
	}
    | expr '*' expr                 {
		$$ = new ast::BinaryExpr(ast::op::MUL, $1, $3);
	}
    | expr '/' expr                 {
		$$ = new ast::BinaryExpr(ast::op::DIV, $1, $3);
	}
    | expr '%' expr                 {
		$$ = new ast::BinaryExpr(ast::op::MOD, $1, $3);
	}
    | '-' expr %prec PREC_UNARY_OP  {
		$$ = new ast::UnaryExpr(ast::op::NEG, $2);
	}
    | '+' expr %prec PREC_UNARY_OP  {
		$$ = new ast::UnaryExpr(ast::op::POS, $2);
	}
    | '!' expr %prec PREC_UNARY_OP  {
		$$ = new ast::UnaryExpr(ast::op::NOT, $2);
	}
    | number                        {
		$$ = new ast::Number($1);
	}
    | TK_IDENT                      {
        $$ = new ast::Id(new std::string(*$1));
    }
;

number
    : TK_INT_CONST
;

%%

void yyerror(ast::CompUnit *&ast, const char *s) {
    throw std::string(s);
}

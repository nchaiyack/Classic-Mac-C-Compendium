%{
  /* C stuff here */
#ifndef __GNUC__
#include "alloca.h"
#endif
#include "globals.h"
int yyparse(void);
extern void yyerror(const char *msg);
extern int yylex(void);

%}

%token T_AND T_ARRAY T_BEGIN T_BOOLEAN T_CHAR T_DIV T_DO
%token T_ELSE T_END T_FALSE T_FUNCTION T_IF
%token T_INTEGER T_MOD T_NOT T_OF T_OR T_PROCEDURE
%token T_PROGRAM T_READ T_REAL T_THEN
%token T_TRUE T_VAR T_WHILE T_WRITE

%token T_LTEQ T_NOTEQ T_LT T_GTEQ T_GT T_EQ
%token T_ASSIGNOP T_DOTDOT

%token T_DIGITS T_NUM T_ID

%nonassoc T_THEN       /* to eliminate shift/reduce error */
%nonassoc T_ELSE

%start program

%%

program: T_PROGRAM T_ID '(' id_list ')' ';'
         declns subprog_declns compound_stmt '.'
        ;
         

id_list:  T_ID
        | id_list ',' T_ID
        | error ','
        ;

declns:   declns T_VAR id_list ':' type ';'
        | /* empty */
        ;

type:     std_type
        | array_prefix T_DOTDOT T_DIGITS ']' T_OF std_type
        | array_prefix    ','   { yyerror("use \"..\" instead of a comma");}
          T_DIGITS ']' T_OF std_type
        ;

array_prefix: T_ARRAY '[' T_DIGITS
        ;

std_type: T_INTEGER | T_REAL | T_BOOLEAN | T_CHAR
        | T_ID { yyerror("illegal data type"); }
        ;

subprog_declns: subprog_declns subprog_decln ';'
        | /* empty */
        ;

subprog_decln: subprog_head declns subprog_declns compound_stmt
        ;

subprog_head: T_FUNCTION T_ID arguments ':' std_type ';'
        |  T_PROCEDURE T_ID arguments ';'
        ;

arguments: '(' parameter_list ')'
        | /* empty */
        ;

parameter_list: id_list ':' type
        | parameter_list ';' id_list ':' type
        | error ';'
        ;

compound_stmt: T_BEGIN stmt_list T_END
        ;

stmt_list: stmt
        |  stmt_list ';' stmt
        ;

stmt:     compound_stmt
        | T_WHILE expression T_DO stmt
        | T_IF expression T_THEN stmt
        | T_IF expression T_THEN stmt T_ELSE stmt
        | T_READ '(' expression_list ')'
        | T_WRITE '(' expression_list ')'
        | T_ID assignop expression            /* expanded "variable" rule */
        | T_ID '[' expression ']' assignop expression
        | T_ID                              /* expanded procedure_stmt rule */
        | T_ID '(' expression ')'
        | T_ID '(' expression ')'
          assignop { yyerror("can't assign a subroutine call a value");}
          expression
        | /* empty */
        ;

assignop: T_ASSIGNOP
        | T_EQ {yyerror("assignment operator required (:=)");}
        ;

/*  These rules became useless when above stmt rule was expanded to
    remove the shift/reduce error from rules 1 and 3 below.  The
    problem was, "does the parser see rule variable or rule procedure_stmt
    after a T_ID is seen?"

variable: T_ID
        | T_ID '[' expression ']'
        ;

procedure_stmt: T_ID
        | T_ID '(' expression ')'
        ;
*/

expression_list: expression
        | expression_list ',' expression
        | error ','
        ;

expression: simple_expr
        | simple_expr relop simple_expr
        ;

simple_expr: term
        | sign term
        | simple_expr addop term
        ;

term:     factor
        | term mulop factor
        ;

factor:   T_ID
        | T_ID '(' expression_list ')'
        | T_ID '[' expression ']'
        | T_NUM
        | T_DIGITS
        | '(' expression ')'
        | '(' ')' { yyerror("empty expressions make no sense"); }
        | T_NOT factor
        | T_TRUE
        | T_FALSE
        ;

sign:     '+'
        | '-'
        ;

relop:    T_EQ    | T_NOTEQ
        | T_LT    | T_LTEQ
        | T_GTEQ  | T_GT
        ;

addop:    sign
        | T_OR
        ;

mulop:    '*'
        | '/'
        | T_DIV
        | T_MOD
        | T_AND
        ;


/* Copyright (c) 1989-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

#ifndef LISP_H
#define LISP_H

/* The different types of lisp objects. */

enum lisptype {
  NIL,
  CONS,
  RAIL,
  NUMBER,
  STRING,
  SYMBOL,
  UTYPE,
  MTYPE,
  TTYPE,
  METHOD,
  EOFOBJ
  };

#define L_NONE 0
#define L_QUOTE 1
#define L_LIST 2
#define L_APPEND 3
#define L_REMOVE 4
/* add booleans etc */

/* Declaration of a cons cell. */

struct a_cons { struct a_obj *car, *cdr; };

/* A symbol includes its index and a pointer to its binding. */

struct a_symbol { int symnum;  struct a_obj *value; };

/* A "method" isn't really code, just an obj with extra word. */

struct a_method { struct a_obj *code; int len; };

/* The basic lisp object.  This should be small. */

typedef struct a_obj {
    enum lisptype type;          /* type of the object */
    union {
	int num;       /* numeric value */
	char *str;     /* string value */
	struct a_symbol sym;
	struct a_cons cons;
	struct a_method meth;
    } v;               /* the "content" of the object */
} Obj;

/* The symbol table is the way to map names into symbols. */

typedef struct a_symentry {
    char *name;
    struct a_obj *symbol;
    char constantp;
} Symentry;

enum strmtype { stringstrm, filestrm };

typedef struct a_strm {
    enum strmtype type;
    union {
	char *sp;
	FILE *fp;
	} ptr;
} Strm;

/* All the lisp interface declarations. */

extern Symentry *symboltable;

extern Obj *lispnil;
extern Obj *lispeof;

Obj *read_form();
Obj *read_form_from_string();
Obj *read_form_aux();
Obj *read_list();
Obj *cons();
Obj *new_string();
Obj *intern_symbol();
Obj *symbol_value();
Obj *setq();
Obj *new_number();
Obj *new_utype();
Obj *new_mtype();
Obj *new_ttype();
Obj *new_method();
Obj *make_namer();
Obj *car(), *cdr(), *cadr(), *cddr(), *caddr();
Obj *eval();
Obj *eval_symbol();
Obj *eval_list();
Obj *elt();

char *c_string();

#endif /* LISP_H */

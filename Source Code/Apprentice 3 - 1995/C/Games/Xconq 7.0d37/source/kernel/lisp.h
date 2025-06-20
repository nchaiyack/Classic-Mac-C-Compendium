/* Definitions for Lisp objects in Xconq.
   Copyright (C) 1989, 1991, 1992, 1993, 1994, 1995 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#ifndef LISP_H
#define LISP_H

/* Lisp objects in Xconq are pretty basic, since they are used only for
   game definition and (usually) not the core computations. */

/* The different types of Lisp objects. */

enum lisptype {
  NIL,
  CONS,
  NUMBER,
  STRING,
  SYMBOL,
  UTYPE,
  MTYPE,
  TTYPE,
  POINTER,
  EOFOBJ
  };

/* Declaration of a cons cell. */

struct a_cons {
    struct a_obj *car;
    struct a_obj *cdr;
};

/* A symbol includes its index and a pointer to its binding. */

struct a_symbol {
    long symnum;
    struct a_obj *value;
};

/* A pointer is an address with associated name.  Interpretation
   and usage is up to the context. */

struct a_pointer {
    struct a_obj *sym;
    char *data;
};

/* The basic Lisp object.  This should be small. */

typedef struct a_obj {
    enum lisptype type;          /* type of the object */
    union {
	int num;       /* numeric value */
	char *str;     /* string value */
	struct a_symbol sym;
	struct a_cons cons;
	struct a_pointer ptr;
    } v;               /* the "content" of the object */
} Obj;

/* The symbol table is the way to map names into symbols. */

typedef struct a_symentry {
    char *name;
    struct a_obj *symbol;
    char constantp;
} Symentry;

/* A stream is just a union of string pointer and file pointer. */

enum strmtype { stringstrm, filestrm };

typedef struct a_strm {
    enum strmtype type;
    union {
	char *sp;
	FILE *fp;
	} ptr;
} Strm;

/* Enum of all the random keywords. */

enum keywords {

#undef  DEF_KWD
#define DEF_KWD(name,CODE,value)  CODE,

#include "keyword.def"

    LAST_KEYWORD
};

#define match_keyword(ob,key) \
  (symbolp(ob) && strcmp(c_string(ob), keyword_name(key)) == 0)

/* All the Lisp interface declarations. */

extern Symentry *symboltable;

extern Obj *lispnil;
extern Obj *lispeof;

extern void init_lisp PROTO ((void));
extern int strmgetc PROTO ((Strm *strm));
extern void strmungetc PROTO ((int ch, Strm *strm));
extern Obj *read_form PROTO ((FILE *fp, int *p1, int *p2));
extern Obj *read_form_from_string PROTO ((char *str, int *p1, int *p2));
extern void sprintf_line_numbers PROTO ((char *buf, int start, int end));
extern Obj *read_form_aux PROTO ((Strm *strm));
extern Obj *read_list PROTO ((Strm *strm));
extern int read_delimited_text PROTO ((Strm *strm, char *delim, int spacedelimits, int eofdelimits));
extern int length PROTO ((Obj *list));
extern Obj *new_string PROTO ((char *str));
extern Obj *new_number PROTO ((int num));
extern Obj *new_utype PROTO ((int u));
extern Obj *new_mtype PROTO ((int r));
extern Obj *new_ttype PROTO ((int t));
extern Obj *new_pointer PROTO ((Obj *sym, char *ptr));
extern Obj *cons PROTO ((Obj *x, Obj *y));
extern void type_warning PROTO ((char *funname, Obj *x, char *typename, Obj *subst));
extern Obj *car PROTO ((Obj *x));
extern Obj *cdr PROTO ((Obj *x));
extern Obj *cadr PROTO ((Obj *x));
extern Obj *cddr PROTO ((Obj *x));
extern Obj *caddr PROTO ((Obj *x));
extern char *c_string PROTO ((Obj *x));
extern int c_number PROTO ((Obj *x));
extern Obj *intern_symbol PROTO ((char *str));
extern int lookup_string PROTO ((char *str));
extern Obj *symbol_value PROTO ((Obj *sym));
extern Obj *setq PROTO ((Obj *sym, Obj *x));
extern void makunbound PROTO ((Obj *sym));
extern void flag_as_constant PROTO ((Obj *sym));
extern int constantp PROTO ((Obj *sym));
extern int numberp PROTO ((Obj *x));
extern int stringp PROTO ((Obj *x));
extern int symbolp PROTO ((Obj *x));
extern int consp PROTO ((Obj *x));
extern int utypep PROTO ((Obj *x));
extern int mtypep PROTO ((Obj *x));
extern int ttypep PROTO ((Obj *x));
extern int pointerp PROTO ((Obj *x));
extern int boundp PROTO ((Obj *sym));
extern int numberishp PROTO ((Obj *x));
extern int equal PROTO ((Obj *x, Obj *y));
extern int member PROTO ((Obj *x, Obj *lis));
extern Obj *elt PROTO ((Obj *lis, int n));
extern void fprintlisp PROTO ((FILE *fp, Obj *obj));
extern void fprint_list PROTO ((FILE *fp, Obj *obj));
extern void sprintlisp PROTO ((char *buf, Obj *obj));
extern void sprint_list PROTO ((char *buf, Obj *obj));
extern void dlisp PROTO ((Obj *x));
extern Obj *append_two_lists PROTO ((Obj *x1, Obj *x2));
extern Obj *append_lists PROTO ((Obj *lis));
extern Obj *remove_from_list PROTO ((Obj *elt, Obj *lis));
extern void push_binding PROTO ((Obj **lis, Obj *key, Obj *val));
extern void push_int_binding PROTO ((Obj **lis, Obj *key, int val));
extern void push_key_binding PROTO ((Obj **lis, int key, Obj *val));
extern void push_key_cdr_binding PROTO ((Obj **lis, int key, Obj *val));
extern void push_key_int_binding PROTO ((Obj **lis, int key, int val));
extern Obj *eval PROTO ((Obj *x));
extern Obj *eval_symbol PROTO ((Obj *x));
extern Obj *eval_list PROTO ((Obj *x));

/* Functions that the Lisp code needs to have defined. */

extern void init_warning PROTO ((char *str, ...));
extern void low_init_warning PROTO ((char *str));
extern void init_error PROTO ((char *str, ...));
extern void low_init_error PROTO ((char *str));
extern void run_warning PROTO ((char *str, ...));
extern void low_run_warning PROTO ((char *str));
extern void run_error PROTO ((char *str, ...));
extern void low_run_error PROTO ((char *str));
extern void announce_read_progress PROTO ((void));
extern int keyword_code PROTO ((char *str));
extern char *keyword_name PROTO ((enum keywords k));
extern int keyword_value PROTO ((enum keywords k));
extern int lazy_bind PROTO ((Obj *sym));
extern void init_predefined_symbols PROTO ((void));

#endif /* LISP_H */

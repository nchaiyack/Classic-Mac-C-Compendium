/* Interface between game parameters and the rest of Xconq.
   Copyright (C) 1992, 1993, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This file defines the structures that are filled in with type info,
   one for each type, plus the declarations for all functions and variables. */

/* Numbers guaranteed to be invalid types in each category.  Should be
   careful that these don't overflow anything. */

#define NONUTYPE (MAXUTYPES)
#define NONMTYPE (MAXMTYPES)
#define NONTTYPE (MAXTTYPES)

/* Indices for each category of types. */

typedef enum {
  UTYP = 0,
  MTYP = 1,
  TTYP = 2
} Typetype;

/* The four roles for terrain. */

enum terrain_subtype {
    cellsubtype = 0,
    bordersubtype = 1,
    connectionsubtype = 2,
    coatingsubtype = 3
};

/* If a specialized Xconq is being compiled, use the specialized .h
   instead of the general one.  All of the general game parameter
   machinery should go away and be replaced by either constant
   defaults or precalculated tables/formulas. */

#ifdef SPECIAL

#include "special.h"

#else

#include "lisp.h"

/* This is the structure representing info about a property
   of a type, such as a unit type's maximum speed. */

typedef struct propertydefn {
    char *name;
    int (*intgetter) PROTO ((int));
    char *(*strgetter) PROTO ((int));
    Obj *(*objgetter) PROTO ((int));
    short offset;
    char *doc;
    short dflt;
    char *dfltstr;
    void (*dlftfn) PROTO ((void));
    short lo, hi;
} PropertyDefn;

/* This is the structure with info about a table. */

typedef struct tabledefn {
    char *name;
    int (*getter) PROTO ((int, int));
    char *doc;
    short **table, *cnst;
    short dflt;
    short lo, hi;
    Typetype index1, index2;
} TableDefn;

/* This is the structure with info about a global variable. */

typedef struct vardefn {
    char *name;
    int (*intgetter) PROTO ((void));
    char *(*strgetter) PROTO ((void));
    Obj *(*objgetter) PROTO ((void));
    char *doc;
    long dflt;
    char *dfltstr;
    void (*dlftfn) PROTO ((void));
    long lo, hi;
} VarDefn;

extern short numutypes;
extern short nummtypes;
extern short numttypes;

typedef struct utype {

#undef  DEF_UPROP_I
#define DEF_UPROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT;
#undef  DEF_UPROP_S
#define DEF_UPROP_S(name,fname,doc,SLOT,dflt)  \
    char *SLOT;
#undef  DEF_UPROP_L
#define DEF_UPROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;

#include "utype.def"

} Utype;

/* Definition of material types. */

typedef struct mtype {

#undef  DEF_MPROP_I
#define DEF_MPROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT;
#undef  DEF_MPROP_S
#define DEF_MPROP_S(name,fname,doc,SLOT,dflt)  \
    char *SLOT;
#undef  DEF_MPROP_L
#define DEF_MPROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;

#include "mtype.def"

} Mtype;

/* Definition of terrain types. */

typedef struct ttype {

#undef  DEF_TPROP_I
#define DEF_TPROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT;
#undef  DEF_TPROP_S
#define DEF_TPROP_S(name,fname,doc,SLOT,dflt)  \
    char *SLOT;
#undef  DEF_TPROP_L
#define DEF_TPROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;

#include "ttype.def"

} Ttype;

/* The global data. */

typedef struct a_globals {

#undef  DEF_VAR_I
#define DEF_VAR_I(name,fname,setfname,doc,VAR,lo,dflt,hi)  \
    long VAR;
#undef  DEF_VAR_S
#define DEF_VAR_S(name,fname,setfname,doc,VAR,dflt)  \
    char *VAR;
#undef  DEF_VAR_L
#define DEF_VAR_L(name,fname,setfname,doc,VAR,dflt)  \
    Obj *VAR;

#include "gvar.def"

} Globals;

/* Declarations of the functions accessing and setting type properties. */

#undef  DEF_UPROP_I
#define DEF_UPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  int FNAME PROTO ((int u));
#undef  DEF_UPROP_S
#define DEF_UPROP_S(name,FNAME,doc,slot,dflt)  char *FNAME PROTO ((int u));
#undef  DEF_UPROP_L
#define DEF_UPROP_L(name,FNAME,doc,slot)  Obj *FNAME PROTO ((int u));

#include "utype.def"

#undef  DEF_MPROP_I
#define DEF_MPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  int FNAME PROTO ((int m));
#undef  DEF_MPROP_S
#define DEF_MPROP_S(name,FNAME,doc,slot,dflt)  char *FNAME PROTO ((int m));
#undef  DEF_MPROP_L
#define DEF_MPROP_L(name,FNAME,doc,slot)  Obj *FNAME PROTO ((int m));

#include "mtype.def"

#undef  DEF_TPROP_I
#define DEF_TPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  int FNAME PROTO ((int t));
#undef  DEF_TPROP_S
#define DEF_TPROP_S(name,FNAME,doc,slot,dflt)  char *FNAME PROTO ((int t));
#undef  DEF_TPROP_L
#define DEF_TPROP_L(name,FNAME,doc,slot)  Obj *FNAME PROTO ((int t));

#include "ttype.def"

#undef  DEF_VAR_I
#define DEF_VAR_I(str,FNAME,SETFNAME,doc,var,lo,dflt,hi)  \
  int FNAME PROTO ((void));  \
  void SETFNAME PROTO ((int val));
#undef  DEF_VAR_S
#define DEF_VAR_S(str,FNAME,SETFNAME,doc,var,dflt)  \
  char *FNAME PROTO ((void));  \
  void SETFNAME PROTO ((char *val));
#undef  DEF_VAR_L
#define DEF_VAR_L(str,FNAME,SETFNAME,doc,var,dflt)  \
  Obj *FNAME PROTO ((void));  \
  void SETFNAME PROTO ((Obj *val));

#include "gvar.def"

/* Declarations of table accessor functions and the globals
   for constant and filled-in tables. */

#undef  DEF_UU_TABLE
#define DEF_UU_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO ((int u1, int u2));  \
  extern short *TABLE, CNST;
#undef  DEF_UM_TABLE
#define DEF_UM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO ((int u, int m));  \
  extern short *TABLE, CNST;
#undef  DEF_UT_TABLE
#define DEF_UT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO ((int u, int t));  \
  extern short *TABLE, CNST;
#undef  DEF_TM_TABLE
#define DEF_TM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO ((int t, int m));  \
  extern short *TABLE, CNST;
#undef  DEF_TT_TABLE
#define DEF_TT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO ((int t1, int t2));  \
  extern short *TABLE, CNST;
#undef  DEF_MM_TABLE
#define DEF_MM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO ((int m1, int m2));  \
  extern short *TABLE, CNST;

#include "table.def"

/* Declarations of the globals description structures. */

extern Globals globals;

extern Utype *utypes;

extern Mtype *mtypes;

extern Ttype *ttypes;

extern PropertyDefn utypedefns[];

extern PropertyDefn mtypedefns[];

extern PropertyDefn ttypedefns[];

extern TableDefn tabledefns[];

extern VarDefn vardefns[];

#endif /* SPECIAL */

/* The following definitions are valid for both general and specialized
   games. */

/* Macros for iterating over types. */

#define for_all_unit_types(v)      for (v = 0; v < numutypes; ++v)

#define for_all_material_types(v)  for (v = 0; v < nummtypes; ++v)

#define for_all_terrain_types(v)   for (v = 0; v < numttypes; ++v)

#define for_all_possible_unit_types(v)      for (v = 0; v < MAXUTYPES; ++v)

#define for_all_possible_material_types(v)  for (v = 0; v < MAXMTYPES; ++v)

#define for_all_possible_terrain_types(v)   for (v = 0; v < MAXTTYPES; ++v)

/* Macros to encapsulate special cases. */

#define could_create(u1,u2) (uu_acp_to_create(u1,u2) > 0)

#define could_repair(u1, u2) (uu_repair(u1,u2) > 0)

/* Fix eventually. */

/* (should say u_... or ..._type ?) */

#define actor(u) (u_acp(u) > 0)

#define mobile(u) (u_speed(u) > 0)

/* backward compat */

#define could_occupy(u,t) \
  (ut_capacity_x(u,t) > 0 || ut_size(u,t) < t_capacity(t))

#define u_hp(u) (u_hp_max(u))

/* Macros for units. */

#define could_move(u,t) (!ut_vanishes_on(u, t) && !ut_wrecks_on(u, t))

#define could_carry(u1,u2)  \
  (uu_capacity_x(u1,u2) > 0 || uu_size(u2,u1) <= u_capacity(u1))

#define could_hit(u1,u2) (uu_hit(u1,u2) > 0)

#define will_garrison(u1, u2) (uu_hp_to_garrison(u1, u2) > 0)

/* These need actual units rather than types. */

#define impassable(u, x, y) (!could_move((u)->type, terrain_at((x), (y))))

#define isbase(u) (u_is_base((u)->type))

#define base_builder(u) (u_is_base_builder((u)->type))

#define istransport(u) (u_is_transport((u)->type))

#define t_is_cell(t) (t_subtype(t) == cellsubtype)

#define t_is_border(t) (t_subtype(t) == bordersubtype)

#define t_is_connection(t) (t_subtype(t) == connectionsubtype)

#define t_is_coating(t) (t_subtype(t) == coatingsubtype)

#define is_unit_type(u) ((u) >= 0 && (u) < numutypes)

#define is_material_type(m) ((m) >= 0 && (m) < nummtypes)

#define is_terrain_type(t) ((t) >= 0 && (t) < numttypes)

extern short tmputype;
extern short tmpmtype;
extern short tmpttype;

extern void checku PROTO ((int x));
extern void utype_error  PROTO ((int u));
extern void checkm PROTO ((int x));
extern void mtype_error  PROTO ((int m));
extern void checkt PROTO ((int x));
extern void ttype_error  PROTO ((int t));
extern void init_types PROTO ((void));
extern void init_globals PROTO ((void));
extern void default_unit_type PROTO ((int x));
extern void default_material_type PROTO ((int x));
extern void default_terrain_type PROTO ((int x));
extern char *index_type_name();
extern void allocate_table PROTO ((int tbl, int reset));
extern int numtypes_from_index_type PROTO ((int x));
extern char *index_type_name PROTO ((int x));

extern void set_g_synth_methods_default PROTO ((void));
extern void set_g_side_lib_default PROTO ((void));
#if 0 /* unused */
extern Obj *get_u_extension PROTO ((int u, char *name, Obj *dflt));
extern Obj *get_m_extension PROTO ((int m, char *name, Obj *dflt));
extern Obj *get_t_extension PROTO ((int t, char *name, Obj *dflt));
#endif

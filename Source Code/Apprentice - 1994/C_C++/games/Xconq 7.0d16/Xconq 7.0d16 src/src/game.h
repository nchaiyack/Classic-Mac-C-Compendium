/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Interface file between game parameters and the rest of Xconq. */

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

typedef struct propertydefn {
    char *name;
    int (*intgetter)();
    char *(*strgetter)();
    Obj *(*objgetter)();
    short offset;
    char *doc;
    short dflt;
    char *dfltstr;
    int (*dlftfn)();
    short lo, hi;
} PropertyDefn;

typedef struct tabledefn {
    char *name;
    int (*getter)();
    char *doc;
    short **table, *cnst;
    short dflt;
    short lo, hi;
    Typetype index1, index2;
} TableDefn;

extern short numutypes;         /* number of unit types */
extern short nummtypes;         /* number of material types */
extern short numttypes;         /* number of terrain types */

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
    short VAR;
#undef  DEF_VAR_S
#define DEF_VAR_S(name,fname,setfname,doc,VAR,dflt)  \
    char *VAR;
#undef  DEF_VAR_L
#define DEF_VAR_L(name,fname,setfname,doc,VAR,dflt)  \
    Obj *VAR;

#include "gvar.def"

} Globals;

/* Declarations of the functions accessing and setting parameters. */

#undef  DEF_UPROP_I
#define DEF_UPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  int FNAME PROTO((int u));
#undef  DEF_UPROP_S
#define DEF_UPROP_S(name,FNAME,doc,slot,dflt)  char *FNAME PROTO((int u));
#undef  DEF_UPROP_L
#define DEF_UPROP_L(name,FNAME,doc,slot)  Obj *FNAME PROTO((int u));

#include "utype.def"

#undef  DEF_MPROP_I
#define DEF_MPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  int FNAME PROTO((int m));
#undef  DEF_MPROP_S
#define DEF_MPROP_S(name,FNAME,doc,slot,dflt)  char *FNAME PROTO((int m));
#undef  DEF_MPROP_L
#define DEF_MPROP_L(name,FNAME,doc,slot)  Obj *FNAME PROTO((int m));

#include "mtype.def"

#undef  DEF_TPROP_I
#define DEF_TPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  int FNAME PROTO((int t));
#undef  DEF_TPROP_S
#define DEF_TPROP_S(name,FNAME,doc,slot,dflt)  char *FNAME PROTO((int t));
#undef  DEF_TPROP_L
#define DEF_TPROP_L(name,FNAME,doc,slot)  Obj *FNAME PROTO((int t));

#include "ttype.def"

#undef  DEF_VAR_I
#define DEF_VAR_I(str,FNAME,SETFNAME,doc,var,lo,dflt,hi)  \
  int FNAME PROTO(());  \
  void SETFNAME PROTO((int val));
#undef  DEF_VAR_S
#define DEF_VAR_S(str,FNAME,SETFNAME,doc,var,dflt)  \
  char *FNAME PROTO(());  \
  void SETFNAME PROTO((char *val));
#undef  DEF_VAR_L
#define DEF_VAR_L(str,FNAME,SETFNAME,doc,var,dflt)  \
  Obj *FNAME PROTO(());  \
  void SETFNAME PROTO((Obj *val));

#include "gvar.def"

#undef  DEF_UU_TABLE
#define DEF_UU_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO((int u1, int u2));  \
  extern short *TABLE, CNST;
#undef  DEF_UM_TABLE
#define DEF_UM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO((int u, int m));  \
  extern short *TABLE, CNST;
#undef  DEF_UT_TABLE
#define DEF_UT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO((int u, int t));  \
  extern short *TABLE, CNST;
#undef  DEF_TM_TABLE
#define DEF_TM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO((int t, int m));  \
  extern short *TABLE, CNST;
#undef  DEF_TT_TABLE
#define DEF_TT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO((int t1, int t2));  \
  extern short *TABLE, CNST;
#undef  DEF_MM_TABLE
#define DEF_MM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi)  \
  int FNAME PROTO((int m1, int m2));  \
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

extern PropertyDefn vardefns[];

extern TableDefn tabledefns[];

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

/* should become "could_create" eventually */
#define could_make(u1,u2) (uu_acp_to_create(u1,u2) > 0)

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

#define could_capture(u1,u2) (uu_capture(u1, u2) > 0)

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

#define um_tomove(u,m) (um_to_move(u,m))
#define um_storage(u,m) (um_storage_x(u,m))
#define um_consume(u,m) (um_base_consumption(u,m))
 
#define is_unit_type(u) ((u) >= 0 && (u) < numutypes)

#define is_material_type(m) ((m) >= 0 && (m) < nummtypes)

#define is_terrain_type(t) ((t) >= 0 && (t) < numttypes)

extern short tmputype;
extern short tmpmtype;
extern short tmpttype;


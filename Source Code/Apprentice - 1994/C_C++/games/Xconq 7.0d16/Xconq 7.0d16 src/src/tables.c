/* Copyright (c) 1992  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Define functions for all the tables. */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "game.h"

/* None of this is used in a compiled Xconq. */

#ifndef SPECIAL

/* Fill in the array that defines the characteristics of each table. */

TableDefn tabledefns[] = {

#undef  DEF_UU_TABLE
#define DEF_UU_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI)  \
    { NAME, FNAME, DOC, &TABLE, &CNST, DFLT, LO, HI, UTYP, UTYP },
#undef  DEF_UM_TABLE
#define DEF_UM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI)  \
    { NAME, FNAME, DOC, &TABLE, &CNST, DFLT, LO, HI, UTYP, MTYP },
#undef  DEF_UT_TABLE
#define DEF_UT_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI)  \
    { NAME, FNAME, DOC, &TABLE, &CNST, DFLT, LO, HI, UTYP, TTYP },
#undef  DEF_TM_TABLE
#define DEF_TM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI)  \
    { NAME, FNAME, DOC, &TABLE, &CNST, DFLT, LO, HI, TTYP, MTYP },
#undef  DEF_TT_TABLE
#define DEF_TT_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI)  \
    { NAME, FNAME, DOC, &TABLE, &CNST, DFLT, LO, HI, TTYP, TTYP },
#undef  DEF_MM_TABLE
#define DEF_MM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI)  \
    { NAME, FNAME, DOC, &TABLE, &CNST, DFLT, LO, HI, MTYP, MTYP },

#include "table.def"

    { NULL }
};

/* Define the accessor functions themselves. */

#undef  DEF_UU_TABLE
#define DEF_UU_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(u1,u2) int u1, u2;  \
    { checku(u1);  checku(u2);  \
      return (TABLE ? TABLE[numutypes*(u1)+(u2)] : CNST); }

#undef  DEF_UM_TABLE
#define DEF_UM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(u,m) int u, m;  \
    { checku(u);  checkm(m);  \
      return (TABLE ? TABLE[nummtypes*(u)+(m)] : CNST); }

#undef  DEF_UT_TABLE
#define DEF_UT_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(u,t) int u, t;  \
    { checku(u);  checkt(t);  \
      return (TABLE ? TABLE[numttypes*(u)+(t)] : CNST); }

#undef  DEF_TM_TABLE
#define DEF_TM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(t,m) int t, m;  \
    { checkt(t);  checkm(m);  \
      return (TABLE ? TABLE[nummtypes*(t)+(m)] : CNST); }

#undef  DEF_TT_TABLE
#define DEF_TT_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(t1,t2) int t1, t2;  \
    { checkt(t1);  checkt(t2);  \
      return (TABLE ? TABLE[numttypes*(t1)+(t2)] : CNST); }

#undef  DEF_MM_TABLE
#define DEF_MM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(m1,m2) int m1, m2;  \
    { checkm(m1);  checkm(m2);  \
      return (TABLE ? TABLE[nummtypes*(m1)+(m2)] : CNST); }

#include "table.def"

#endif /* n SPECIAL */

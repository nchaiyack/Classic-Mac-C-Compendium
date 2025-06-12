/* Copyright (c) 1991-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "game.h"

#ifndef SPECIAL

#ifdef offsetof
#define OFFS(TYPE,FLD) (offsetof(TYPE, FLD))
#else
#define OFFS(TYPE,FLD)  \
  ((unsigned int) (((char *) (&(((TYPE *) NULL)->FLD))) - ((char *) NULL)))
#endif

PropertyDefn utypedefns[] = {

#undef  DEF_UPROP_I
#define DEF_UPROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Utype, SLOT), DOC, DFLT, NULL, NULL, LO, HI },
#undef  DEF_UPROP_S
#define DEF_UPROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Utype, SLOT), DOC,    0, DFLT, NULL,  0,  0 },
#undef  DEF_UPROP_L
#define DEF_UPROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Utype, SLOT), DOC,    0, NULL, NULL,  0,  0 },

#include "utype.def"

    { NULL }
};

PropertyDefn mtypedefns[] = {

#undef  DEF_MPROP_I
#define DEF_MPROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Mtype, SLOT), DOC, DFLT, NULL, NULL, LO, HI },
#undef  DEF_MPROP_S
#define DEF_MPROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Mtype, SLOT), DOC,    0, DFLT, NULL,  0,  0 },
#undef  DEF_MPROP_L
#define DEF_MPROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Mtype, SLOT), DOC,    0, NULL, NULL,  0,  0 },

#include "mtype.def"

    { NULL }
};

PropertyDefn ttypedefns[] = {

#undef  DEF_TPROP_I
#define DEF_TPROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Ttype, SLOT), DOC, DFLT, NULL, NULL, LO, HI },
#undef  DEF_TPROP_S
#define DEF_TPROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Ttype, SLOT), DOC,    0, DFLT, NULL,  0,  0 },
#undef  DEF_TPROP_L
#define DEF_TPROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Ttype, SLOT), DOC,    0, NULL, NULL,  0,  0 },

#include "ttype.def"

    { NULL }
};

#undef  DEF_UPROP_I
#define DEF_UPROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(u) int u; { checku(u); return utypes[u].SLOT; }
#undef  DEF_UPROP_S
#define DEF_UPROP_S(name,FNAME,doc,SLOT,dflt)  \
  char *FNAME(u) int u; { checku(u); return utypes[u].SLOT; }
#undef  DEF_UPROP_L
#define DEF_UPROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(u) int u; { checku(u); return utypes[u].SLOT; }

#include "utype.def"

/* Define functions for all material type properties. */

#undef  DEF_MPROP_I
#define DEF_MPROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(m) int m; { checkm(m); return mtypes[m].SLOT; }
#undef  DEF_MPROP_S
#define DEF_MPROP_S(name,FNAME,doc,SLOT,dflt)  \
  char *FNAME(m) int m; { checkm(m);  return mtypes[m].SLOT; }
#undef  DEF_MPROP_L
#define DEF_MPROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(m) int m; { checkm(m);  return mtypes[m].SLOT; }

#include "mtype.def"

/* Define functions for all terrain type properties. */

#undef  DEF_TPROP_I
#define DEF_TPROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(t) int t; { checkt(t); return ttypes[t].SLOT; }
#undef  DEF_TPROP_S
#define DEF_TPROP_S(name,FNAME,doc,SLOT,dflt)  \
  char *FNAME(t) int t; { checkt(t); return ttypes[t].SLOT; }
#undef  DEF_TPROP_L
#define DEF_TPROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(t) int t; { checkt(t); return ttypes[t].SLOT; }

#include "ttype.def"

#endif

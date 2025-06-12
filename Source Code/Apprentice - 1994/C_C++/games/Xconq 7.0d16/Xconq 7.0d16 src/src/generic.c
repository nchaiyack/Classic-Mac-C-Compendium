/* Copyright (c) 1991-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Defines parameter formulas. */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "game.h"

int set_g_side_lib_default();
int set_g_synth_methods_default();

/* The total number of unit/material/terrain types. */

short numutypes;
short nummtypes;
short numttypes;

/* Doublechecks on type indices.  These can show up all kinds of little
   glitches. */

/* It would be useful for these to return the slot or table access that
   resulted in the error, but we don't want to have to pass a bunch of
   parameters to these routines, since they're being called a lot. */

checku(x)
int x;
{
    if ((x) < 0 || (x) >= numutypes) utype_error(x);
}

utype_error(u)
int u;
{
    run_warning("Bad utype %d", u);
}

checkm(x)
int x;
{
    if ((x) < 0 || (x) >= nummtypes) mtype_error(x);
}

mtype_error(r)
int r;
{
    run_warning("Bad mtype %d", r);
}

checkt(x)
int x;
{
    if ((x) < 0 || (x) >= numttypes) ttype_error(x);
}

ttype_error(t)
int t;
{
    run_warning("Bad ttype %d", t);
}

#ifdef SPECIAL

/* When specially compiled, all the types and globals are statically
   initialized. */

init_types()
{
}

init_globals()
{
}

#else /* SPECIAL */

/* Declarations of the type definitions themselves. */

Utype *utypes;

Mtype *mtypes;

Ttype *ttypes;

Globals globals;

/* This prepares the type definitions to be filled in, doing initial
   allocations, etc. */

int curmaxutypes = MAXUTYPES;
int curmaxmtypes = MAXMTYPES;
int curmaxttypes = MAXTTYPES;

init_types()
{
    numutypes = nummtypes = numttypes = 0;

    Dprintf("Utype is %d bytes, mtype is %d bytes, ttype is %d bytes.\n",
	    sizeof(Utype), sizeof(Mtype), sizeof(Ttype));

    utypes = (Utype *) xmalloc(sizeof(Utype) * curmaxutypes);
    mtypes = (Mtype *) xmalloc(sizeof(Mtype) * curmaxmtypes);
    ttypes = (Ttype *) xmalloc(sizeof(Ttype) * curmaxttypes);

    bzero(utypes, sizeof(Utype) * curmaxutypes);
    bzero(mtypes, sizeof(Mtype) * curmaxmtypes);
    bzero(ttypes, sizeof(Ttype) * curmaxttypes);
}

PropertyDefn vardefns[] = {

#undef  DEF_VAR_I
#define DEF_VAR_I(NAME,FNAME,SETFNAME,DOC,VAR,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, 0, DOC, DFLT, NULL, NULL, LO, HI },
#undef  DEF_VAR_S
#define DEF_VAR_S(NAME,FNAME,SETFNAME,DOC,VAR,DFLT)  \
    { NAME, NULL, FNAME, NULL, 0, DOC,    0, DFLT, NULL,  0,  0 },
#undef  DEF_VAR_L
#define DEF_VAR_L(NAME,FNAME,SETFNAME,DOC,VAR,DFLT)  \
    { NAME, NULL, NULL, FNAME, 0, DOC,    0, NULL, DFLT,  0,  0 },

#include "gvar.def"

    { NULL }
};

/* Define all the global-getting and -setting functions. */

#undef  DEF_VAR_I
#define DEF_VAR_I(str,FNAME,SETFNAME,doc,VAR,lo,dflt,hi)  \
  int FNAME() { return globals.VAR; }  \
  void SETFNAME(v) int v; { globals.VAR = v; }
#undef  DEF_VAR_S
#define DEF_VAR_S(str,FNAME,SETFNAME,doc,VAR,dflt)  \
  char *FNAME() { return globals.VAR; }  \
  void SETFNAME(v) char *v; { globals.VAR = v; }
#undef  DEF_VAR_L
#define DEF_VAR_L(str,FNAME,SETFNAME,doc,VAR,DFLT)  \
  Obj *FNAME() { int (*fn)() = (DFLT);  \
    if (fn != NULL && globals.VAR == NULL) (*fn)();  \
    return globals.VAR; }  \
  void SETFNAME(v) Obj *v; { globals.VAR = v; }

#include "gvar.def"

/* Set the globals to their default values. */

init_globals()
{

	/* (should zero out all the globals, only set the nonzero defaults) */

#undef  DEF_VAR_I
#define DEF_VAR_I(str,fname,SETFNAME,doc,var,lo,DFLT,hi)  \
    if ((DFLT) != 0 || 1 /* for now */) SETFNAME(DFLT);
#undef  DEF_VAR_S
#define DEF_VAR_S(str,fname,SETFNAME,doc,var,DFLT)  \
    if ((DFLT) != NULL || 1 /* for now */) SETFNAME(DFLT);
#undef  DEF_VAR_L
#define DEF_VAR_L(str,fname,SETFNAME,doc,var,DFLT)  \
    if ((DFLT) == NULL) SETFNAME(lispnil);

#include "gvar.def"

}

#define TYPEPROP(TYPES, N, DEFNS, I, TYPE)  \
  ((TYPE *) &(((char *) (&(TYPES[N])))[DEFNS[I].offset]))[0]

/* This sets all the defaults in a unit type definition.  Note that all
   type structures get blasted with zeros initially, so we really only
   need to do default settings of nonzero values, thus the test.  (It
   helps if the compiler is smart enough to remove dead code.) */

default_unit_type(u)
int u;
{
    int i, offset;

    for (i = 0; utypedefns[i].name != NULL; ++i) {
	if (utypedefns[i].intgetter) {
	    if (utypedefns[i].dflt != 0)
	      TYPEPROP(utypes, u, utypedefns, i, short) = utypedefns[i].dflt;
	} else if (utypedefns[i].strgetter) {
	    if (utypedefns[i].dfltstr != NULL)
	      TYPEPROP(utypes, u, utypedefns, i, char *) = (char *) utypedefns[i].dfltstr;
	} else {
	    TYPEPROP(utypes, u, utypedefns, i, Obj *) = lispnil;
	}
    }
}

/* This sets all the defaults in a material type definition. */

default_material_type(m)
int m;
{
    int i;
	
    for (i = 0; mtypedefns[i].name != NULL; ++i) {
	if (mtypedefns[i].intgetter) {
	    if (mtypedefns[i].dflt != 0)
	      TYPEPROP(mtypes, m, mtypedefns, i, short) = mtypedefns[i].dflt;
	} else if (mtypedefns[i].strgetter) {
	    if (mtypedefns[i].dfltstr != NULL)
	      TYPEPROP(mtypes, m, mtypedefns, i, char *) = (char *) mtypedefns[i].dfltstr;
	} else {
	    TYPEPROP(mtypes, m, mtypedefns, i, Obj *) = lispnil;
	}
    }
}

/* This sets all the defaults in a terrain type definition. */

default_terrain_type(t)
int t;
{
    int i;
	
    for (i = 0; ttypedefns[i].name != NULL; ++i) {
	if (ttypedefns[i].intgetter) {
	    if (ttypedefns[i].dflt != 0)
	      TYPEPROP(ttypes, t, ttypedefns, i, short) = ttypedefns[i].dflt;
	} else if (ttypedefns[i].strgetter) {
	    if (ttypedefns[i].dfltstr != 0)
	      TYPEPROP(ttypes, t, ttypedefns, i, char *) = (char *) ttypedefns[i].dfltstr;
	} else {
	    TYPEPROP(ttypes, t, ttypedefns, i, Obj *) = lispnil;
	}
    }
}

#endif /* SPECIAL */

char *
index_type_name(x)
int x;
{
    return ((x) == UTYP ? "unit" : ((x) == MTYP ? "material" : "terrain"));
}

/* This function allocates a parameter table and fills it with a default. */

allocate_table(tbl, reset)
int tbl, reset;
{
    int i, lim1, lim2, dflt = tabledefns[tbl].dflt;
    short *rslt;
    extern int canaddutype, canaddmtype, canaddttype;

    if (reset) *(tabledefns[tbl].table) = NULL;
    if (*tabledefns[tbl].table == NULL) {
	lim1 = numtypes_from_index_type(tabledefns[tbl].index1);
	lim2 = numtypes_from_index_type(tabledefns[tbl].index2);
	if (lim1 == 0) {
	    run_warning("Can't allocate the %s table, no %s types defined",
			tabledefns[tbl].name, index_type_name(tabledefns[tbl].index1));
	    return;
	}
	if (lim2 == 0) {
	    run_warning("Can't allocate the %s table, no %s types defined",
			tabledefns[tbl].name, index_type_name(tabledefns[tbl].index2));
	    return;
	}
	/* Allocate the table itself. */
	rslt = (short *) xmalloc(lim1 * lim2 * sizeof(short));
	/* Put the table's default everywhere in the table. */
	for (i = 0; i < lim1 * lim2; ++i) rslt[i] = dflt;
	*(tabledefns[tbl].table) = rslt;
	/* For each index, flag that no more types of that sort allowed. */
	switch (tabledefns[tbl].index1) {
	  case UTYP: canaddutype = FALSE;  break;
	  case MTYP: canaddmtype = FALSE;  break;
	  case TTYP: canaddttype = FALSE;  break;
	}
	switch (tabledefns[tbl].index2) {
	  case UTYP: canaddutype = FALSE;  break;
	  case MTYP: canaddmtype = FALSE;  break;
	  case TTYP: canaddttype = FALSE;  break;
	}
    }
}

numtypes_from_index_type(x)
int x;
{
    return ((x) == UTYP ? numutypes : ((x) == MTYP ? nummtypes : numttypes));
}

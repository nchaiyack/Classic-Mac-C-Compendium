/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Random definitions used everywhere. */

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

/* This is how we do optional prototypes. */

#ifndef PROTO
#ifdef __STDC__
#define PROTO(ARGS) ARGS
#else
#define PROTO(ARGS) ()
#endif /* __STDC__ */
#endif /* PROTO */

#define abs(x) (((x) < 0) ? (0 - (x)) : (x))

#ifndef min
#define min(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef max
#define max(x,y) (((x) > (y)) ? (x) : (y))
#endif

#define between(lo,n,hi) ((lo) <= (n) && (n) <= (hi))

#define flip_coin() (xrandom(257) % 2)

#define avg(a,b) (((a) + (b)) / 2)

#ifndef isspace
#define isspace(c) ((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\r')
#endif

#define lowercase(c) (isupper(c) ? tolower(c) : (c))

#define uppercase(c) (islower(c) ? toupper(c) : (c))

/* This tests a string to see if it has anything in it. */

#define empty_string(s) ((s) == NULL || s[0] == '\0')

#ifndef USE_CONSOLE

/* Attempt to disable console I/O. */

#undef  printf
#define printf nothing
#undef  scanf
#define scanf abort

#undef  stdin
#define stdin -1
#undef  stdout
#define stdout -1
#undef  stderr
#define stderr -1

#endif

extern int curturn;
extern int cursequence;

extern char spbuf[];
extern char tmpbuf[];
extern char *plural_form();
extern char *ordinal_suffix();
extern char *copy_string();
extern char *read_line();
extern char *version;
extern char *xconqlib;
extern void init_xrandom();
extern int xrandom();
extern char *getenv();

extern FILE *errorfp;

#ifdef DEBUGGING

/* Debugging definitions. */

#define Dprintf if (Debug) debug_printf
#define DMprintf if (DebugM) debugm_printf
#define DGprintf if (DebugG) debugg_printf

#define Dprintlisp(X) if (Debug) fprintlisp(dfp, (X))
#define DMprintlisp(X) if (DebugM) fprintlisp(dmfp, (X))
#define DGprintlisp(X) if (DebugG) fprintlisp(dgfp, (X))

/* If the debug flags are not macros, then declare them as globals. */

#ifndef Debug
extern int Debug;
#endif
#ifndef DebugM
extern int DebugM;
#endif
#ifndef DebugG
extern int DebugG;
#endif

extern FILE *dfp;
extern FILE *dmfp;
extern FILE *dgfp;

#else /* DEBUGGING */

/* need to make defns and calls vanish if possible */

#endif /* DEBUGGING */

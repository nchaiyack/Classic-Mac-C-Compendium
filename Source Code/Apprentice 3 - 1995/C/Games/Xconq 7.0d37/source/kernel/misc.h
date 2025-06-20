/* Random definitions used everywhere in Xconq.
   Copyright (C) 1987, 1988, 1989, 1991, 1992, 1993, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

/* This is how we do optional prototypes and const decls. */

#ifdef MSDOS
#define PROTO(ARGS) ARGS
#define CONST const
#endif

#ifndef PROTO
#ifdef __STDC__
#define PROTO(ARGS) ARGS
#else
#define PROTO(ARGS) ()
#endif /* __STDC__ */
#endif /* PROTO */

#ifndef CONST
#ifdef __STDC__
#define CONST const
#else
#define CONST
#endif /* __STDC__ */
#endif /* CONST */

#ifndef ABS
#define ABS(x) (((x) < 0) ? (0 - (x)) : (x))
#endif

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

extern char spbuf[];
extern char tmpbuf[];

#ifdef DEBUGGING

#ifndef USE_CONSOLE

/* Attempt to disable console I/O. */

#undef  printf
#define printf abort
#undef  scanf
#define scanf abort

#undef  stdin
#define stdin (-1)
#undef  stdout
#define stdout (-1)
#undef  stderr
#define stderr (-1)

#endif /* ! USE_CONSOLE */

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

/* Make defns and calls vanish if possible. */

#define Dprintf if (0) debug_printf
#define DMprintf if (0) debugm_printf
#define DGprintf if (0) debugg_printf

#define Dprintlisp(X)
#define DMprintlisp(X)
#define DGprintlisp(X)

#define Debug (0)
#define DebugM (0)
#define DebugG (0)

#define dfp stdout
#define dmfp stdout
#define dgfp stdout

#endif /* DEBUGGING */

extern char *getenv();

extern void init_xrandom PROTO ((int seed));
extern int xrandom PROTO ((int m));
extern int probability PROTO ((int prob));
extern int roll_dice PROTO ((int n));
extern int prob_fraction PROTO ((int n));

extern char *xmalloc PROTO ((int amt));
extern void report_malloc PROTO ((void));
extern void tprintf PROTO ((char *buf, char *str, ...));
extern void tnprintf PROTO ((char *buf, int n, char *str, ...));
extern char *copy_string PROTO ((char *str));
extern int iindex PROTO ((int ch, char *str));
extern long idifftime PROTO ((time_t t1, time_t t0));
extern void case_panic PROTO ((char *str, int var));
extern int isqrt PROTO ((int i));
extern void profile_printf PROTO ((char *str, ...));
extern void debug_printf PROTO ((char *str, ...));
extern void debugm_printf PROTO ((char *str, ...));
extern void debugg_printf PROTO ((char *str, ...));

extern void vtprintf PROTO ((char *buf, char *str, va_list ap));

extern void make_pathname PROTO ((char *path, char *name, char *extn,
				  char *pathbuf));

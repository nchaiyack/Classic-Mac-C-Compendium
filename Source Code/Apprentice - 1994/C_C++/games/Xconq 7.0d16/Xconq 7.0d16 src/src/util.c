/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Random utilities not classifiable elsewhere. Many are not Xconq-specific. */

#include "conq.h"

/* To accommodate certain obsolete (pre-ANSI) Unix systems. */
#ifndef time_t
#define time_t long
#endif

/* Direction-to-delta conversions. */

int dirx[] = DIRX;
int diry[] = DIRY;

/* Random number handling is important to game but terrible/nonexistent
   in some systems.  Do it ourselves and hope it's better... */

/* The random state *must* be at least 32 bits. */

long initrandstate = -1;

long randstate = -1;

/* Seed can come from elsewhere, for repeatability.  Otherwise, it comes
   from the current time, scaled down to where 32-bit arithmetic won't
   overflow. */

void
init_xrandom(seed)
int seed;
{
    time_t tm;
    	
    if (seed >= 0) {
	/* If the random state is already set, changes are somewhat
	   suspicious. */
	if (randstate >= 0) {
	    Dprintf("Randstate being changed from %d to %d\n",
		    randstate, seed);
	}
	randstate = seed;
    } else {
#ifdef MPW
	/* what to do here? */
#else
    	time(&tm);
#endif
    	randstate = (long) tm;
    }
    /* Whatever its source, put the randstate into known range (0 - 99999). */
    randstate = abs(randstate);
    randstate %= 100000L;
    /* This is kept around for the sake of error reporting. */
    initrandstate = randstate;
}

/* Numbers lifted from Numerical Recipes, p. 198. */
/* Arithmetic must be 32-bit. */

xrandom(m)
int m;
{
    randstate = (8121 * randstate + 28411) % 134456L;
    return ((m * randstate) / 134456L);
}

/* Percentage probability, with bounds checking. */

probability(prob)
int prob;
{
    if (prob <= 0) return FALSE;
    if (prob >= 100) return TRUE;
    return (xrandom(100) < prob);
}

roll_dice(n)
int n;
{
    int numdice, dice, i, rslt;

    if (n >> 14 == 0 || n >> 14 == 3) return n;
    numdice = (n >> 11) & 0x07;
    dice = (n >> 7) & 0x0f;
    rslt = n & 0x7f;
    for (i = 0; i < numdice; ++i) {
	rslt += xrandom(dice);
    }
    return rslt;
}

prob_fraction(n)
int n;
{
	return (n / 100 + (probability(n % 100) ? 1 : 0));
}

#ifdef DEBUGGING

/* This tracks our total space allocation. */

#define NUMMALLOCRECS 100

int overflow = 0;

int totmalloc = 0;

int nextmalloc = 0;

int copymalloc = 0;

int grandtotmalloc = 0;

struct a_malloc {
    int size;
    int count;
} *mallocs = NULL;

/* Given an amount of memory allocated, record it with the others. */

record_malloc(amt)
int amt;
{
    int i;

    /* Might need to allocate the record of allocations. */
    if (mallocs == NULL) {
	/* Don't use xmalloc here!! */
	mallocs = (struct a_malloc *)
	  malloc(NUMMALLOCRECS * sizeof(struct a_malloc));
	overflow = 0;
    }
    /* Search for already-recorded allocations of same-size blocks. */
    for (i = 0; i < nextmalloc; ++i) {
	if (amt == mallocs[i].size) {
	    ++(mallocs[i].count);
	    return;
	}
    }
    if (nextmalloc < NUMMALLOCRECS) {
	/* Record this allocation as a new size of allocation. */
	mallocs[nextmalloc].size = amt;
	mallocs[nextmalloc].count = 1;
	++nextmalloc;
    } else {
    	/* Add to the overflow bucket, which is used for allocations whose
	   sizes could not be recorded individually. */
    	overflow += amt;
    }
}

malloc_record_compare(m1, m2)
struct a_malloc *m1, *m2;
{
    return (m1->size * m1->count - m2->size * m2->count);
}

/* Display memory consumption and reset records.  This does not account
   for freeing, but Xconq usually tries to hang onto and reuse anything
   it allocates, very rarely calls free(). */

void
report_malloc()
{
    extern int lispmalloc, numsymbols, maxsymbols;
    int i;

    if (nextmalloc == 0) {
	Dprintf("No mallocs since last report.\n");
	return;
    }
    Dprintf("Mallocs since last report:\n");
    Dprintf("    Amount =  Bytes x  Times\n");
    /* Sort the entries. */
    qsort(mallocs, nextmalloc, sizeof(struct a_malloc), malloc_record_compare);
    /* Write out all the records, formatting nicely. */
    for (i = 0; i < nextmalloc; ++i) {
	Dprintf("%10d = %6d x %6d\n",
		mallocs[i].size * mallocs[i].count,
		mallocs[i].size,  mallocs[i].count);
    }
    if (overflow > 0) Dprintf("%10d other\n", overflow);
    Dprintf("Total malloced = %d bytes.\n", totmalloc);
    Dprintf("String copies = %d bytes.\n", copymalloc);
    Dprintf("Lisp malloced = %d bytes.\n", lispmalloc);
    Dprintf("%d/%d symbols interned.\n", numsymbols, maxsymbols);
    Dprintf("Grand total allocation = %d bytes.\n", grandtotmalloc);
    /* Reset all the counters for next time. */
    nextmalloc = overflow = 0;
    totmalloc = copymalloc = lispmalloc = 0;
}

#endif /* DEBUGGING */

/* This is our improved and adapted version of malloc, that guarantees
   zeroing of the block, checks for memory exhaustion, and collects
   usage statistics. */

char *
xmalloc(amt)
int amt;
{
    char *value = (char *) malloc(amt);

    if (value == NULL) {
	/* This is pretty serious, have to get out quickly. */
	run_error("Memory exhausted!!");
	/* In case run_error doesn't exit. */
  	exit(1);
    }
    /* Save callers from having to clear things themselves. */
    bzero(value, amt);
#ifdef DEBUGGING
    /* This can't be controlled by `Debug', because much allocation
       happens before any command line or dialog options are interpreted. */
    totmalloc += amt;
    grandtotmalloc += amt;
    record_malloc(amt);
#endif
    return value;
}

/* Copy a string to newly-allocated space.  The new space is never freed. */

char *
copy_string(str)
char *str;
{
    int len = strlen(str);
    char *rslt;

    rslt = xmalloc(len + 1);
    strcpy(rslt, str);
#ifdef DEBUGGING
    copymalloc += len + 1;
#endif
    return rslt;
}

/* Get a *numeric* index into a string (more useful than ptr, in Xconq).
   Return -1 on failed search. */

iindex(ch, str)
char ch, *str;
{
    int i;

    if (str == NULL) return (-1);
    for (i = 0; str[i] != '\0'; ++i) if (ch == str[i]) return i;
    return (-1);
}

/* Return a time difference as an long integer number of seconds. */

long
idifftime(t1, t0)
time_t t1, t0;
{
#ifdef MAC /* (should be anywhere that actually has a difftime function) */
    return ((long) difftime(t1, t0));
#else
    return ((long) t1 - (long) t0);
#endif
}

/* This little routine goes at the end of all switch statements on internal
   data values. */

case_panic(str, var)
char *str;
int var;
{
    run_error("Panic! Unknown %s %d", str, var);
}

/* Integer square root - good enough, no float trickery or libs needed. */

/* Improved version from Bruce Fast, via Scott Herod. */

int
isqrt(i)
int i;
{
    int j, k;

    if (i > 3) {
        for (j = i, k = -1; j >>= 2; k <<= 1);
        k = (~k + i / ~k) / 2;
	k = (k + i / k) / 2;
	k = (1 + k + i / k) / 2;
	return (k + i / k) / 2;
    } else if (i > 0) {
	return 1;
    } else {
	return 0;
    }
}

#ifdef DEBUGGING

/* Debugging flags.
   These are set up so that they can be macros or variables, as desired. */

#ifndef Debug
int Debug = FALSE;
#endif
#ifndef DebugM
int DebugM = FALSE;
#endif
#ifndef DebugG
int DebugG = FALSE;
#endif

/* These are where normal debugging output goes. */

FILE *pfp;
FILE *dfp;
FILE *dgfp;
FILE *dmfp;

/* Kind of cheesy, should only use if real ANSI not available. */

profile_printf(str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)
char *str;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
{
    fprintf(pfp, str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

debug_printf(str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)
char *str;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
{
    fprintf(dfp, str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

debugm_printf(str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)
char *str;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
{
    fprintf(dmfp, str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

debugg_printf(str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)
char *str;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
{
    fprintf(dgfp, str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

#endif /* DEBUGGING */

/* This exists to provide a function placeholder. */

nothing() {}

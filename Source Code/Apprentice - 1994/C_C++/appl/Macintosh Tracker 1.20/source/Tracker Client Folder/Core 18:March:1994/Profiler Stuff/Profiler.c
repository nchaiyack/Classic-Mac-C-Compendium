#if __option(profile)

/*
 *  profile.c
 *
 *  Copyright (c) 1991 Symantec Corporation.  All rights reserved.
 *
 */

/* Warning:  This file has been modified to suit my purposes! */

#pragma options(!profile)

#include "stdio.h"
#include "limits.h"
#include "stdlib.h"
#include "string.h"
#include "Profiler.h"

/*
 *  #define _VIATIMER_ to use VIA ticks rather than clock ticks.
 *
 *  Clock ticks occur 60 times a second, whereas VIA ticks occur
 *  approximately 780,000 times a second.  Therefore VIA ticks yield
 *  a more accurate profile, though they are more sensitive to
 *  interrupt-level activity.  Also the VIA timer is used by the
 *  Sound Driver, and therefore may conflict with some programs.
 *
 */

#ifdef _VIATIMER_
#define Ticks	VIA_ticks()
#endif

/*  symbol table entry  */
static struct sym {
	unsigned char	*fname;
	long			count;
	long			total;
	long			min;
	long			max;
} *syms;

/*  stack entry  */
static struct stack {
	struct sym		*sym;
	void			**ret;
	long			start;
	long			overhead;
} *stack;

int _profile, _trace;
static unsigned nsyms, depth, max_nsyms, max_depth;

void _profile_(unsigned char *);
static struct sym *lookup(unsigned char *);
static void *profile_exit(void);
static void done(void);
static int compare(const void *, const void *);


/*
 *  InitProfile
 *
 *  This must be called once to initialize the profiler.  The arguments
 *  specify the maximum number of functions to be profiled, and the
 *  maximum number of nested function calls.
 *
 */

FILE*	tracefile;

void
InitProfile(unsigned nsyms, unsigned depth)
{
	if (syms = malloc(nsyms * (long) sizeof(struct sym)))
		max_nsyms = nsyms;
	if (stack = malloc(depth * (long) sizeof(struct stack)))
		max_depth = depth;
	_profile = true;
/*	_trace = true; */
	if (_trace)
		tracefile = fopen(" Trace Dump","w");
}


/*
 *  _profile_ - profiler entry point
 *
 *  Each function compiled with the "Profile" option begins with a call
 *  to _profile_("\pfuncname").
 *
 */

void
_profile_(unsigned char *fname)
{
	register struct sym *p;
	register struct stack *q;
	long start;
	int tracing;
	
	if (_profile && depth < max_depth) {
		start = Ticks;
		tracing = _trace;
		_profile = _trace = false;
		if (p = lookup(fname)) {
			if (tracing)
				fprintf(tracefile,"%*s%#s\n", depth, "", fname);
			++p->count;
			q = &stack[depth++];
			q->sym = p;
			asm {
				movea.l	(a6),a0
				move.l	4(a0),q->ret
				lea		@exit,a1
				move.l	a1,4(a0)
			}
			q->start = start;
			q->overhead = Ticks - start;
		}
		_profile = true;
		_trace = tracing;
	}
	return;

		/*  the profiled function will return here  */
		
exit:
	asm {
		move.l	d0,-(sp)		;  preserve result
		jsr		profile_exit
		movea.l	d0,a0			;  real return address
		move.l	(sp)+,d0		;  restore result
		jmp		(a0)			;  return
	}
}


/*
 *  profile_exit
 *
 *  "_profile_" arranges for this routine to be called as each profiled
 *  function returns.
 *
 */

static void *
profile_exit(void)
{
	register struct stack *q = &stack[--depth];
	register struct sym *p = q->sym;
	register long gross = Ticks - q->start;
	register long net = gross - q->overhead;

	if (net < 0)
		net = 0;
	if (p->max < net)
		p->max = net;
	if (p->min > net)
		p->min = net;
	p->total += net;
	if (depth)
		q[-1].overhead += gross;
	return(q->ret);
}


/*
 *  lookup - get the symbol table entry for the current function
 *
 */

static struct sym *
lookup(unsigned char *fname)
{
	register unsigned i = 0, j, n = nsyms;
	register struct sym *p;
	register struct stack *q;
	
	if (n == 0)
		atexit(done);
		
		/*  binary search  */
		
	while (i < n) {
		j = (i + n - 1) >> 1;
		p = &syms[j];
		if (p->fname == fname)
			return(p);
		if (p->fname > fname)
			n = j;
		else
			i = j + 1;
	}
	
		/*  insert new symbol into table  */
		
	if (nsyms == max_nsyms)
		return(NULL);
	p = &syms[i];
	memmove(p + 1, p, (nsyms - i) * (long) sizeof(struct sym));
	p->fname = fname;
	p->count = p->total = p->max = 0;
	p->min = LONG_MAX;
	++nsyms;
	
		/*  adjust pointer to moved symbols  */
		
	for (q = stack, n = depth; n--; q++) {
		if (q->sym >= p)
			++q->sym;
	}
	return(p);
}


/*
 *  DumpProfile - print a report
 *
 *  It is not necessary to call this routine, as it will be called
 *  automatically at program exit.
 *
 */

void
DumpProfile(void)
{
	int profiling = _profile, tracing = _trace;
	register unsigned n;
	register struct sym *p;
	register long total = 0;
	FILE*	DumpFile;

	DumpFile = fopen(" Profile Dump","w");
	_profile = _trace = false;
	fprintf(DumpFile,"\n%-47s  Minimum  Maximum  Average    %%   Entries\n\n", "Function");
	for (p = syms, n = nsyms; n--; p++)
		total += p->total;
	for (p = syms, n = nsyms; n--; p++) {
		fprintf(DumpFile,"%#-47s%8lu %8lu %8lu   %3u %8lu\n",
					p->fname,
					p->min == LONG_MAX ? 0 : p->min,
					p->max,
					p->count ? p->total / p->count : 0,
					total ? (int) (100. * p->total / total) : 0,
					p->count);
	}
	fclose(DumpFile);
	if (_trace)
		fclose(tracefile);
	_profile = profiling;
	_trace = tracing;
}


/*
 *  done - print a report automatically at exit
 *
 */

static void
done(void)
{
	_profile = _trace = false;
	qsort(syms, nsyms, sizeof(struct sym), compare);
	DumpProfile();
}


/*
 *  compare - comparison routine for sorting symbols alphabetically
 *
 */

static int
compare(const void *P, const void *Q)
{
	register const struct sym *p = P;
	register const struct sym *q = Q;
	int np = p->fname[0], nq = q->fname[0], result;
	
	result = strncmp((char *) &p->fname[1], (char *) &q->fname[1], np < nq ? np : nq);
	return(result ? result : (np - nq));
}

#endif

/* MacMemory.h */

#pragma once

#ifndef _STDLIB
#include <stdlib.h>
#endif

#include	"memman.h"
/*
void *mcalloc(size_t nelem, size_t nsize);
void mfree(void *p);
void *mmalloc(size_t n);
void *mrealloc(void *ptr, size_t size);
*/

// now we use my entirely rewritten ANSI-like
// memory management routines [Fabrizio Oddone]

#define	mcalloc(n, s)	fcalloc(n, s)
#define	mfree(p)	ffree(p)
#define	mmalloc(s)	fmalloc(s)
#define	mrealloc(p, s)	frealloc(p, s)

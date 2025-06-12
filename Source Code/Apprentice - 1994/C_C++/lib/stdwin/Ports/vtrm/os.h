/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1991. */

/* Auto-configuration file for vtrm.
   Edit this if you have portability problems. */

#include <stdio.h>
#include <ctype.h>
#include <string.h>		/* Or <strings.h> for 4.2 BSD */

/* is this a termio system ? */
#ifdef SYSV
#define HAS_TERMIO
#endif

/* 4.2 BSD select() system call available ? */
#ifndef SYSV
#define HAS_SELECT
#endif

/* can #include <signal.h> ? */
#define SIGNAL

#ifdef SIGNAL
#define SIGTYPE void
/* type returned by signal handler function: (used to be int) */
#endif

/* can #include <setjmp.h> ? */
#define SETJMP

/* VOID is used in casts only, for quieter lint */
/* make it empty if your compiler doesn't have void */
#define VOID (void)

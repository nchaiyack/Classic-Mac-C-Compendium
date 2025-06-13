/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file manages the define symbols which handle conditional compilation.
 * 
 */

#ifndef CONDITCOMP
#define CONDITCOMP

/* Change the #defines here */

#define NDEBUG 1

#ifdef OLDMEM
#define P__H *
#else
#define P__H 
#endif

#define Via(x) (P__H(x))
typedef char P__H*EString_t;

#endif

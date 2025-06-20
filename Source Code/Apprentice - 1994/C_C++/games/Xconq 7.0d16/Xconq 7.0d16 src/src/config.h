/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This file has many things that can be fiddled with.  As distributed,
   no changes should be required to get Xconq running on any vaguely
   BSD-like Unix system.  (But of course there are always exceptions
   that I've never heard of before!)

   Everything here is graphics-independent, each interface type foo should
   have a <foo>.h that defines its config options. */

/* (should try to flush remainder of sys details, move to interface?) */

/* #define UNIX /* Normally defined on command line or in makefile */

/* #define BSD  /* Normally defined on command line or in makefile */

/* #define MAC  /* Normally defined in project preferences. */

/* Universally required include files. */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#ifdef UNIX

#ifdef BSD

#define strchr index
#define strrchr rindex

#else /* probably SVRn? */

#define bzero(x,y) memset(x,0,y)

#endif /* BSD */

#endif /* UNIX */

#ifdef MAC

#define bzero(x,y) memset(x,0,y)

#endif

/* Deliberately break any attempts to call rand() or random().
   See defn of xrandom() for more explanation. */

#define rand() abort()
#define random(x) abort()

/* This is where game modules can be found. */
/* This is a fallback, defn usually comes from Makefile. */

#ifndef XCONQLIB
#ifdef UNIX
#define XCONQLIB "../lib"
#endif /* UNIX */
#ifdef MAC
#define XCONQLIB ":lib"
#endif /* MAC */
#endif /* XCONQLIB */

/* The newsfile always lives in the lib directory. */

#define NEWSFILE "xconq.news"

/* Default random world size.  Adjust these to taste - for the standard
   game, 60x60 is a moderate length game, 30x30 is short, 360x120 is
   L-O-N-G !  (Remember to multiply the two numbers to get an idea of the
   number of hexes that will be in the world!)  Defaults can also be set
   by modules explicitly, so this is really a "default default" :-) . */

#define DEFAULTWIDTH 60
#define DEFAULTHEIGHT 30

/* Default world circumference.  If this matches the default width, then
   the game area wraps around in x; otherwise it will be a hexagon. */

#define DEFAULTCIRCUMFERENCE 360

/* Absolute maximum number of sides that can play.  This should never be
   set higher than 30.  Smaller values will incur fewer assorted memory
   penalties, and in any case games with lots of sides don't usually
   play very well. */

#define MAXSIDES 10

/* Absolute maximum number of kinds of units.  The lower this is set, the
   less memory needed (although most allocation actually works off the actual
   number of types, so effect is minor), but 126 is absolutely the top
   unless you want to do lots of rewriting. */

#define MAXUTYPES 126

/* Maximum number of material/supply types.  Limits same as for unit types. */

#define MAXMTYPES 126

/* Maximum number of terrain types.  Limits same as for unit types. */

#define MAXTTYPES 126

/* Some parts of the code rarely need the full magnitude of the MAX*TYPE
   values, so these values are used to cut down average allocation.  They
   can be adjusted to favor the kinds of game designs that you usually run. */

#define INITMAXUTYPES 20
#define INITMAXMTYPES  5
#define INITMAXTTYPES 15

/* All names, phrases, and messages must be able to fit in statically
   allocated buffers of this size. */

#define BUFSIZE 120

/* The following options are (should be anyway) mostly independent of each
   other, and settable at will. */

/* When defined, save files will be removed after the game starts
   up successfully.  This keeps junk from lying around, but some
   folks prefer that they be kept just in case. */

#define RMSAVE

/* If this is on, messages will be somewhat nastier.  Set this to be
   appropriate for your intended audience, heh-heh :-) */

#define RUDE

/* This sets a limit on how many previous views will be retained.
   Each increment adds 4*area bytes of allocation. */

#define MAXVIEWHISTORY 0

/* If defined, then an editing mode is available.  This is also a powerful
   way to cheat, which is why it's optional. */

#define DESIGNERS

/* If defined, some (actually a lot) debugging code will be included. */

#define DEBUGGING

/* If defined, the compiler will be included. */

#define COMPILER

/* This is true if we try to reuse dead units. */

/* #define REUSE_DEAD */


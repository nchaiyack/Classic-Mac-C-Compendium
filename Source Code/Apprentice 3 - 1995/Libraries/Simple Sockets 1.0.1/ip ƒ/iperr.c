/*
 * iperr.c
 *
 * Global Error globals
 * In the future, more interesting error handlers might go here.
 *
 * Mike Trent 8/94
 */

#include <Types.h>
#include "iperr.h"	/* unneeded, but fun */

/* global to everyone */

int		gErrno;
OSErr	gMacErrno;
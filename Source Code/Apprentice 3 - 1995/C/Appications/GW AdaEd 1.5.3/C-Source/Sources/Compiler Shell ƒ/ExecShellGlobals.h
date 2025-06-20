/*****
 * ExecShellGlobals.h
 *****/
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */


#pragma once

#ifndef __SETJMP__
#include <setjmp.h>
#endif

#ifndef _STDIO
#include <stdio.h>
#endif

/* Global variables */
extern	WindowPtr	gShellWindow;
extern	Rect		gShellDragRect;
extern	FILE		*gConsole;

extern	long		gShellStartupTime;		/* time when this application started */
extern	Boolean		gShellQuitting;
extern	jmp_buf		gJumpEnv;				/* environment for jump */

extern	Boolean		gProcessing;
extern	Rect		gProgressBar;
extern	Rect		gProgressArea;
extern	MenuHandle	gAppleMenu, gFileMenu, gEditMenu;

extern	char		*gCmdLineOptions;

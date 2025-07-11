/*****
 * ExecShellGlobals.c
 *****/
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */

#include <console.h>
#include <setjmp.h>
#include <stdio.h>


#include "ExecShellGlobals.h"

/* Global variables */
WindowPtr	gShellWindow;
Rect		gShellDragRect;
FILE		*gConsole;

long		gShellStartupTime;		/* time when this application started */
Boolean		gShellQuitting;
jmp_buf		gJumpEnv;				/* environment for jump */

Boolean		gProcessing;
Rect		gProgressBar;
Rect		gProgressArea;
MenuHandle	gAppleMenu, gFileMenu, gEditMenu;

char		*gCmdLineOptions;

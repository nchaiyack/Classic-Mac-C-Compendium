/*
 * ShellGlobals.h
 */

#ifndef __SETJMP__
#include <setjmp.h>
#endif

#pragma once

/* Global variables */
extern	WindowPtr	gShellWindow;

extern	long		gShellStartupTime;		/* time when this application started */

/* read-write - set this if you want the shell to terminate
 * execution once the compiler is finished with its work.
 * If set to false, the shell will continue executing until
 * the user selects Quit from the File Menu.
 */

extern	Boolean		gShellQuitting;


extern	jmp_buf		gJumpEnv;				/* environment for jump */

extern	Boolean		gProcessing;
extern	Rect		gProgressBar;
extern	Rect		gProgressArea;
extern	MenuHandle	gAppleMenu, gFileMenu, gEditMenu;

extern	char		*gCmdLineOptions;


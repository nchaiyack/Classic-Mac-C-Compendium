/*
 * ShellGlobals.c
 */


#include <setjmp.h>

/* Global variables */
WindowPtr	gShellWindow;

long		gShellStartupTime;		/* time when this application started */
Boolean		gShellQuitting;
jmp_buf		gJumpEnv;				/* environment for jump */

Boolean		gProcessing;
Rect		gProgressBar;
Rect		gProgressArea;
MenuHandle	gAppleMenu, gFileMenu, gEditMenu;

char		*gCmdLineOptions;


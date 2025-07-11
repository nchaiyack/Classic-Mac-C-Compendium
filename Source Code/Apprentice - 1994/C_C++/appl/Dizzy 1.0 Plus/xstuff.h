/*
>>	This file contains the X dependent defines for Dizzy
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	You are entitled to do whatever you wish with this program, as long as
>>	you give credit all those people, who have worked on this program. If
>>	you are going to sell this program or obtain commercial benefit from it
>>	otherwise, please consult the authors first to obtain information about
>>	licensing this program for commercial use.
>>
*/

#ifdef	XMAIN_PROGRAM
#define XGLOBAL
#else
#define XGLOBAL extern
#endif

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <X11/StringDefs.h>

XGLOBAL Display 	*Disp;
XGLOBAL GC			Mode;
XGLOBAL Pixel		Black,White;
XGLOBAL Window		Wind;
XGLOBAL XFontStruct *MyFont;
XGLOBAL Widget		Canvas;
XGLOBAL Widget		Toplevel;
XGLOBAL Arg 		Wargs[10];
XGLOBAL Pixmap		MenuPix;
XGLOBAL Pixmap		ToolPix;
XGLOBAL Pixmap		MiscPix;
XGLOBAL Pixmap		SplashPix;
XGLOBAL XEvent		MyEvent;
XGLOBAL int 		DownTrigger;
XGLOBAL int 		MainDisable;				/*	Disables clicks in main window. */
XGLOBAL char		CurFileName[MAXFILENAME];	/*	Let's hope paths get no longer! */
XGLOBAL char		*DestFileName;

#define INITIAL_WIDTH	500
#define INITIAL_HEIGHT	400

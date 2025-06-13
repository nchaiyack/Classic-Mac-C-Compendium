/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:	TextEditor.h

Description:
	This file contains typedefs, function prototypes and extern var declarations
	that need to be exported from texteditor.c.
------------------------------------------------------------------------------
Authors:
	Jim Nitchals, Eduard [esp] Schwan
------------------------------------------------------------------------------
	from Persistence of Vision Raytracer
	Copyright 1993 Persistence of Vision Team
------------------------------------------------------------------------------
	NOTICE: This source code file is provided so that users may experiment
	with enhancements to POV-Ray and to port the software to platforms other 
	than those supported by the POV-Ray Team.  There are strict rules under
	which you are permitted to use this file.  The rules are in the file
	named POVLEGAL.DOC which should be distributed with this file. If 
	POVLEGAL.DOC is not available or for more info please contact the POV-Ray
	Team Coordinator by leaving a message in CompuServe's Graphics Developer's
	Forum.  The latest version of POV-Ray may be found there as well.

	This program is based on the popular DKB raytracer version 2.12.
	DKBTrace was originally written by David K. Buck.
	DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
------------------------------------------------------------------------------
More Info:
	This Macintosh version of POV-Ray was created and compiled by Jim Nitchals
	(Think 5.0) and Eduard Schwan (MPW 3.2), based (loosely) on the original
	port by Thomas Okken and David Lichtman, with some help from Glenn Sugden.

	For bug reports regarding the Macintosh version, you should contact:
	Eduard [esp] Schwan
		CompuServe: 71513,2161
		Internet: jl.tech@applelink.apple.com
		AppleLink: jl.tech
	Jim Nitchals
		Compuserve: 73117,3020
		America Online: JIMN8
		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
		AppleLink: JIMN8
------------------------------------------------------------------------------
Change History:
	921110	[esp]	Created
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
==============================================================================*/

#if !defined (TEXTEDITOR_H)
#define TEXTEDITOR_H


/*==== POV headers ====*/
#include "PovMac.h"


/*==== Mac toolbox headers ====*/
// NOTE: _H_MacHeaders_ is defined by Think C if it is using
// precompiled headers.  This is only for compilation speed improvement.
#if !defined(_H_MacHeaders_)
#include <Controls.h>	// ControlHandle
#include <Dialogs.h>	// DialogPtr
#include <textedit.h>	// TEHandle
#endif // _H_MacHeaders_


/*==== External globals ====*/

extern WindowPtr		gSrcWind_Window;
extern TEHandle			gSrcWind_TEH;
extern Boolean			gSrcWind_dirty;
extern Boolean			gSrcWind_visible;
extern Str255 			gSrcWind_FileName;
extern short			gSrcWind_VRefNum;
extern ControlHandle 	gSrcWind_VScroll;


/*==== TextEditor.c Prototypes ====*/

int		SetUpFiles(void);
int		DoFile(int item);
int		SaveAs(Str255 fn, short *vRef);
int		SaveFile(Str255 fn, short vRef);
int		NewFile(Str255 fn, short *vRef);
int		OldFile(Str255 fn, short *vRef);
int		CreateFile(Str255 fn, short *vRef, short *theRef);
int		WriteFile(short refNum, char *p, long num);
int		ReadFile(short refNum, TEHandle textH);
int		pStrCopy(StringPtr p1, StringPtr p2);
int		FileError(Str255 s, Str255 f);
void	PreInitWindows(void);
int		SetUpWindows(void);
int		AdjustText(void);
int		SetVScroll(void);
int		ShowSelect(void);
void	SelectAllText(void);
int		SetView(WindowPtr w);
int		UpdateWindow(WindowPtr theWindow);
pascal void ScrollProc(ControlHandle theControl, short theCode);
int		DoContent(WindowPtr theWindow, EventRecord *theEvent);
void	MyResizeWindow(WindowPtr w, short h, short v);
int		CloseMyWindow(void);
int		main_init(void);
int		DoEditMouseDown(int windowPart, WindowPtr whichWindow, EventRecord *myEvent);
int		MaintainCursor(void);


#endif // TEXTEDITOR_H

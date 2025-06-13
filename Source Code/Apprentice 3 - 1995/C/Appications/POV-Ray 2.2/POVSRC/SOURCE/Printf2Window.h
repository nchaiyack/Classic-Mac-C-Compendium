/*
==============================================================================
Project:	POV-Ray

Version:	2.2

File Name:	Printf2Window.h

Description:
	General-purpose printf-capturing routines that allow a console-like
	output window for c programs that otherwise prefer to use printf/fprintf.
	This code was "inspired heavily" from sources such as MacDTS'es TESample,
	MacApp's Transcript window, and previous code of mine.  It is fairly well
	self-contained, and works in MPW C 3.2 and Think C 5.0.

	This file contains global definitions used by the p2w routines.
	It is expected to be included only by source files that are
	Macintosh-toolbox-aware.

Related Files:
	Stdio_p2w.h		- generic header for sources that would otherwise use <stdio.h>
	Printf2Window.h	- Mac-specific header for p2w routines
	Printf2Window.c	- the main source for the p2w routines
------------------------------------------------------------------------------
Author:
	Eduard [esp] Schwan
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
	920318	[esp]	Created.
	920325	[esp]	Added prototypes for Std C routines
	920330	[esp]	Updated file header with copyright & related files info
	920401	[esp]	Added p2wSignature to window record for safety checking
	920529	[esp]	Changed type defs to have trailing _t for ANSI consistency
	920621	[esp]	Changed file header definition to _PRINTF2WINDOW_H_ for ANSI consistency
	920816	[esp]	Added p2w_SelectAll routine
	920912	[esp]	Added windID parm to NewWindow call
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
==============================================================================
*/

#if !defined(PRINTF2WINDOW_H)
#define PRINTF2WINDOW_H


// include our minimal stuff

#include "stdio_p2w.h"

#include <types.h>		// Boolean
#include <windows.h>	// WindowRecord
#include <textedit.h>	// TEHandle


// ==== Extended p2w window record

typedef struct
	{
	WindowRecord	p2wWindowRec;		// our window/grafport record
	OSType			p2wSignature;		// Magic value that identifies this as a true p2w window
	Boolean			p2wOpenedOK;		// true if p2wWindowRec is valid, for p2w_DisposeWindow
	TEHandle		p2wTEHandle;		// the TE record we use
	ControlHandle	p2wVScroller;		// the vertical scrollbar in the window
	ControlHandle	p2wHScroller;		// the horizontal scrollbar in the window
	ProcPtr			p2wClickHandler;	// the autoscroll routine (not yet)
	short			p2wMaxDocWidth;		// TE record's max wrap width
	Boolean			p2wAlwaysScrollToBottom;	// true means always stay scrolled to bottom of TE
	}
	p2w_WindowRecord_t,
	*p2w_WindowPtr_t,
	**p2w_WindowHandle_t;



// ==== Public p2w library routines

OSErr p2w_Init(void);

OSErr p2w_Terminate(void);

p2w_WindowPtr_t p2w_NewWindow
					(const	short				windID,
					const	Rect				*windBounds,
					const	Str255				windTitle,
					const	Boolean				windIsVisible,
					const	int					windFont,
					const	short				windFontSize,
							OSErr				*anError);

void p2w_DisposeWindow
					(		p2w_WindowPtr_t		the_p2wPtr);

void p2w_SetTextFont
					(		p2w_WindowPtr_t		the_p2wPtr,
							short				newFontType,
							short				newFontSize);

OSErr p2w_AddCString
					(		p2w_WindowPtr_t		the_p2wPtr,
					const	char				*theCStrPtr);

void p2w_DrawWindow
					(const	p2w_WindowPtr_t		the_p2wPtr);

void p2w_DoUpdate
					(const	p2w_WindowPtr_t		thep2wWindow);

void p2w_DoActivate
					(const	p2w_WindowPtr_t		thep2wWindow,
							Boolean				becomingActive);

void p2w_DoGrow
					(const	p2w_WindowPtr_t		thep2wWindow,
							EventRecord			*theEvent);

void p2w_DoZoom
					(const	p2w_WindowPtr_t		thep2wWindow,
							short				thePart);

void p2w_SelectAll
					(const	p2w_WindowPtr_t		thep2wWindow);

void p2w_DoContentClick
					(const	p2w_WindowPtr_t		thep2wWindow,
							EventRecord			*theEvent);


#endif // PRINTF2WINDOW_H
